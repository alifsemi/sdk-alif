import groovy.json.JsonOutput
import groovy.json.JsonSlurper

private def request(String method, String path, Object body = null, String token) {
    String github_api = "https://api.github.com"
    String json       = body ? JsonOutput.toJson(body).replace("'", "'\\''") : ""
    String data       = body ? "-d ${json}" : ""
    String cmd        = ""

    cmd += "curl -sS "
    cmd += "-X ${method} "
    cmd += "-H 'Accept: application/vnd.github+json' "
    cmd += "-H 'X-GitHub-Api-Version: 2026-03-10' "

    if (body != null) {
        cmd += "${data} "
    }

    cmd += ' "' + github_api + path + '"'

    def response = sh(
        script: """\
            set +x
            ${cmd} -H "Authorization: Bearer \$GITHUB_TOKEN" -w "\\nHTTP_STATUS:%{http_code}"
        """,
        returnStdout: true
    ).trim()

    if (!response) {
        echo "Empty response from GitHub"
        return [:]
    }

    def lines    = response.readLines()
    def status   = lines[-1].replace("HTTP_STATUS:", "").trim()
    def respBody = lines.size() > 1 ? lines[0..-2].join("\n") : ""

    int httpStatus = status.toInteger()

    if (httpStatus < 200 || httpStatus >= 300) {
        echo "WARNING: GitHub request API failed. HTTP ${status}"
        echo "Response: ${respBody}"
    } else {
        echo "HTTP Status: ${status}"
    }

    try {
        return new JsonSlurper().parseText(respBody)
    } catch (Exception e) {
        echo "Response is not JSON"
        return respBody
    }
}

def createPullRequest(Map a) {
    String body= a.body ?: ""
    String argJson = '{' +
        '"title": "' + a.title + '", ' +
        '"head" : "' + a.head  + '", ' +
        '"base" : "' + a.base  + '", ' +
        '"body" : "' + a.body  + '"'   +
        '}'

    request(
        "POST",
        "/repos/${a.owner}/${a.repo}/pulls",
        argJson,
        a.token
    )
}

def getPullRequest(Map a) {
    request(
        "GET",
        "/repos/${a.owner}/${a.repo}/pulls/${a.changeId}",
        null,
        a.token
    )
}

def listPullRequests(Map a) {
    def prs     = []
    int page    = 1
    int perPage = 100

    while (true) {
        def git_api_path =
            "/repos/${a.owner}/${a.repo}/pulls?state=${a.state ?: 'open'}&per_page=${perPage}&page=${page}"

        echo "Fetching PR page: ${page}"

        def result = request(
            "GET",
            git_api_path,
            null,
            a.token
        )

        if (!(result instanceof List)) {
            echo "Unexpected response while fetching PRs"
            return prs
        }

        echo "PRs returned in page ${page}: ${result.size()}"

        for (def pr : result) {
            prs << [
                changeId: pr.number,
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
    request(
        "PATCH",
        "/repos/${a.owner}/${a.repo}/pulls/${a.changeId}",
        a.body,
        a.token
    )
}

def getPullRequestTitle(Map a) {
    def result = request(
        "GET",
        "/repos/${a.owner}/${a.repo}/pulls/${a.changeId}",
        null,
        a.token
    )

    if (result instanceof Map && result.title != null) {
        return result.title.toString()
    }

    return ""
}

def getPullRequestBody(Map a) {
    def result = request(
        "GET",
        "/repos/${a.owner}/${a.repo}/pulls/${a.changeId}",
        null,
        a.token
    )

    if (result instanceof Map && result.body != null) {
        return result.body.toString()
    }

    return ""
}

def addLabel(Map a) {
    String argJson = '[ "' + a.label + '" ]'
    request(
        "POST",
        "/repos/${a.owner}/${a.repo}/issues/${a.changeId}/labels",
        argJson,
        a.token
    )
}

def removeLabel(Map a) {
    request(
        "DELETE",
        "/repos/${a.owner}/${a.repo}/issues/${a.changeId}/labels/${a.label}",
        null,
        a.token
    )
}

def getPRlabelsList(Map a) {
    request(
        "GET",
        "/repos/${a.owner}/${a.repo}/issues/${a.changeId}/labels",
        null,
        a.token
    )
}

def addComment(Map a) {
    request(
        "POST",
        "/repos/${a.owner}/${a.repo}/issues/${a.changeId}/comments",
        ['"body" : "' + a.comment + '"'],
        a.token
    )
}

def updateComment(Map a) {
    request(
        "PATCH",
        "/repos/${a.owner}/${a.repo}/issues/comments/${a.commentId}",
        ['"body" : "' + a.comment + '"'],
        a.token
    )
}

def deleteComment(Map a) {
    request(
        "DELETE",
        "/repos/${a.owner}/${a.repo}/issues/comments/${a.commentId}",
        null,
        a.token
    )
}

def getPullRequestSha(Map a) {
    def result = request(
        "GET",
        "/repos/${a.owner}/${a.repo}/pulls/${a.changeId}",
        null,
        a.token
    )

    return result?.head?.sha
}

def getPullRequestCommitMessage(Map a) {
    def result = request(
        "GET",
        "/repos/${a.owner}/${a.repo}/pulls/${a.changeId}/commits?per_page=100",
        null,
        a.token
    )

    if (!(result instanceof List) || result.isEmpty()) {
        return null
    }

    return result[-1]?.commit?.message
}

def getPullRequestMergeSHA(Map a) {
    def result = request(
        "GET",
        "/repos/${a.owner}/${a.repo}/git/ref/pull/${a.changeId}/merge",
        null,
        a.token
    )
    return result?.object?.sha
}

return this
