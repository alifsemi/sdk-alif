def request(String method, String path, String body = null,
        String token, int log = 1, List<Integer> allowedStatuses = [])
{
    String github_api = "https://api.github.com"
    String cmd        = "curl -sS -X ${method} "
    String cmdLog     = log < 1 ? 'set +x' : ''

    cmd += "-H 'Accept: application/vnd.github+json' "
    cmd += "-H 'X-GitHub-Api-Version: 2026-03-10' "

    if (body != null) {
        cmd += '--data-binary "$REQUEST_BODY" '
    }

    cmd += "\"${github_api}${path}\""

    def response = withEnv(["REQUEST_BODY=${body ?: ''}"]) {
        sh(script: """#!/bin/bash
        ${cmdLog}
        ${cmd} -H "Authorization: Bearer \$GITHUB_TOKEN" \
        -w "\\nHTTP_STATUS:%{http_code}"
        """, returnStdout: true).trim()
    }

    if (!response) {
        error "GitHub API returned an empty response"
    }

    def lines    = response.readLines()
    def status   = lines[-1].replace("HTTP_STATUS:", "").trim()
    def respBody = lines.size() > 1 ? lines[0..-2].join("\n") : ""

    int httpStatus = status.toInteger()

    // 2xx is always successful.
    // Explicitly supplied statuses are also allowed.
    boolean success = (httpStatus >= 200 && httpStatus < 300) ||
        allowedStatuses.contains(httpStatus)

    if (!success) {
        echo "GitHub API failed. HTTP ${httpStatus}"
        echo "Response: ${respBody}"
        error "GitHub API request failed: HTTP ${httpStatus}"
    }

    if (log >= 1) {
        echo "HTTP Status: ${httpStatus}"
    }

    // DELETE normally returns 204
    if (httpStatus == 204 || !respBody) {
        return [:]
    }

    try {
        return readJSON(text: respBody)
    } catch (Exception e) {
        echo "Response is not JSON"
        return respBody
    }
}

def createPullRequest(Map a) {
    String body = a.body ?: ""
    def argJson = withEnv(["T=${a.title}", "H=${a.head}", "B=${a.base}", "BODY=${body}"]) {
        sh(script: '''#!/bin/bash
            set +x
            jq -n --arg t "$T" --arg h "$H" --arg b "$B" --arg body "$BODY" \
            '{title:$t, head:$h, base:$b, body:$body}'
            ''', returnStdout: true).trim()
    }
    request("POST", "/repos/${a.owner}/${a.repo}/pulls", argJson, a.token)
}

def getPullRequest(Map a) {
    request("GET", "/repos/${a.owner}/${a.repo}/pulls/${a.prNumber}",
        null, a.token)
}

def listPullRequests(Map a) {
    def prs     = []
    int page    = 1
    int perPage = 100

    while (true) {
        def git_api_path =
            "/repos/${a.owner}/${a.repo}/pulls?state=${a.state ?: 'open'}&per_page=${perPage}&page=${page}"

        echo "Fetching PR page: ${page}"

        def result = request("GET", git_api_path, null, a.token)

        if (!(result instanceof List)) {
            echo "Unexpected response while fetching PRs"
            return prs
        }

        echo "PRs returned in page ${page}: ${result.size()}"

        for (def pr : result) {
            prs << [    prNumber: pr.number,
                        body    : pr.body?.toString(),
                        title   : pr.title?.toString(),
                        state   : pr.state?.toString()
            ]
        }

        // Less than 100 means this was the last page
        if (result.size() < perPage) {
            break
        }
        page++
    }

    echo "Total PRs collected: ${prs.size()}"
    return prs
}

def updatePullRequest(Map a) {
    request("PATCH", "/repos/${a.owner}/${a.repo}/pulls/${a.prNumber}", a.body, a.token)
}

def getPullRequestTitle(Map a) {
    def result = request("GET", "/repos/${a.owner}/${a.repo}/pulls/${a.prNumber}",
            null, a.token)

    if (result instanceof Map && result.title != null) {
        return result.title.toString()
    }

    return ""
}

def getPullRequestBody(Map a) {
    def result = request("GET", "/repos/${a.owner}/${a.repo}/pulls/${a.prNumber}",
            null, a.token)

    if (result instanceof Map && result.body != null) {
        return result.body.toString()
    }

    return ""
}

def addLabel(Map a) {
    String argJson = sh(script: """#!/bin/bash
                set +x; jq -n --arg l '${a.label}' '[\$l]'
                """, returnStdout: true).trim()
    request("POST", "/repos/${a.owner}/${a.repo}/issues/${a.prNumber}/labels",
        argJson, a.token)
}

def removeLabel(Map a) {
    request("DELETE", "/repos/${a.owner}/${a.repo}/issues/${a.prNumber}/labels/${a.label}",
        null, a.token, 1, [404])
}

def getPRlabelsList(Map a) {
    request("GET", "/repos/${a.owner}/${a.repo}/issues/${a.prNumber}/labels",
        null, a.token)
}

def addComment(Map a) {
    String argJson = withEnv(["COMMENT_BODY=${a.comment ?: ''}"]) {
        sh(script: '''#!/bin/bash
            set +x; jq -n --arg c "$COMMENT_BODY" '{body:$c}'
            ''', returnStdout: true).trim()
    }
    request("POST", "/repos/${a.owner}/${a.repo}/issues/${a.prNumber}/comments",
        argJson, a.token)
}

def updateComment(Map a) {
    String argJson = sh(script: """#!/bin/bash
            set +x; jq -n --arg c '${a.comment}' '{body:\$c}'
            """,returnStdout: true).trim()
    request("PATCH", "/repos/${a.owner}/${a.repo}/issues/comments/${a.commentId}",
        argJson, a.token)
}

def getCommentIDs(Map a, String commentToBeSearch) {
    def commentIds = []

    def getAllComments = request("GET",
        "/repos/${a.owner}/${a.repo}/issues/${a.prNumber}/comments", null, a.token)

    if (!(getAllComments instanceof List)) {
        echo "Failed to get comments for PR #${a.prNumber} from GitHub"
        return []
    }

    getAllComments.each { comment ->
        if (comment.body?.contains(commentToBeSearch)) {
            commentIds << comment.id
        }
    }

    return commentIds
}

def deleteComment(Map a) {
    request("DELETE", "/repos/${a.owner}/${a.repo}/issues/comments/${a.commentId}",
        null, a.token)
}

def getPullRequestSha(Map a) {
    def result = request("GET", "/repos/${a.owner}/${a.repo}/pulls/${a.prNumber}",
        null, a.token)

    return result?.head?.sha
}

def getPullRequestCommitMessage(Map a) {
    def result = request("GET",
        "/repos/${a.owner}/${a.repo}/pulls/${a.prNumber}/commits?per_page=100",
        null, a.token)

    if (!(result instanceof List) || result.isEmpty()) {
        return null
    }

    return result[-1]?.commit?.message
}

def getPRallFiles(Map a, String prNumber) {
    def allFiles = []
    int page     = 1
    int perPage  = 100

    while (true) {
        def files = request("GET",
            "/repos/${a.owner}/${a.repo}/pulls/${prNumber}/files?per_page=${perPage}&page=${page}",
            null, a.token, 0)
        if (!(files instanceof List) || files.isEmpty()) {
            break
        }

        allFiles.addAll(files)
        if (files.size() < perPage) {
            break
        }
        page++
    }

    return allFiles
}

def findPRWithSpecificText(Map a, String searchString) {
    def matchingPRs = []
    def allFiles    = []
    def prs         = listPullRequests(a)

    for (def pr : prs) {
        def prNumber = pr.prNumber
        echo "Checking #PR-${prNumber}"
        def files = getPRallFiles(a, prNumber.toString())

        for (def file : files) {
            def patch = file.patch ?: ""
            if (file.filename == "west.yml" &&
                patch.contains(searchString)) {
                echo "Found '${searchString}' in #PR-${pr.prNumber}"
                echo "File: ${file.filename}"
                matchingPRs << [
                    prNumber: pr.prNumber,
                    title   : pr.title,
                    filename: file.filename,
                    patch   : patch
                ]
                echo "MatchingPRs collected: ${matchingPRs.size()}"
                // if need a list incase of multiple PRs check, remove
                // this return statement, here in first match only
                // it will return.
                return matchingPRs
            }
        }
    }
    echo "Total Matching PRs collected: ${matchingPRs.size()}"
    return matchingPRs
}

return this
