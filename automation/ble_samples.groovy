def initialize() {
    sh '''#!/bin/bash -xe
        env
        cd /root/alif
        west forall -c "git clean -fdx"
        cd /root/alif/alif/
        git status
        git fetch origin -pu
        if [[ -v CHANGE_ID ]]; then
            git branch -D pr-${CHANGE_ID} || true
            git fetch origin pull/${CHANGE_ID}/head:pr-${CHANGE_ID}
            git checkout pr-${CHANGE_ID}
        else
            git checkout main
            git reset --hard origin/main
            git pull
        fi
        cd ..
        west update -n
    '''
}

def verify_checkpatch(){
    int status = sh(script: '''#!/bin/bash -xe
        cd /root/alif/alif/
        if [[ -v CHANGE_ID ]]; then
            ../zephyr/scripts/checkpatch.pl --ignore=GERRIT_CHANGE_ID,EMAIL_SUBJECT,COMMIT_MESSAGE,COMMIT_LOG_LONG_LINE -g pr-\${CHANGE_ID}...origin/main
            STATUS=\$?
            if [ \$STATUS -ne 0 ]; then
                exit \$STATUS
            else
                echo "Checkpatch passed successfully"
            fi
        fi
        ''',
        returnStatus: true
    )

    return status
}

def verify_gitlint (){
    int status = sh(script: '''#!/bin/bash -xe
        env
        cd /root/alif/alif/
        if [[ -v CHANGE_ID ]]; then
            pip install gitlint
            git rev-list origin/main..HEAD | xargs -r -n1 gitlint --commit
        fi
        exit $?
        ''',
        returnStatus: true
    )

    return status
}

def is_docs_only_change() {
    if (!env.CHANGE_ID) {
        echo "Not a PR build (no CHANGE_ID) -> running full build/test."
        return false
    }

    def changedFiles = sh(
        script: '''#!/bin/bash -xe
            cd /root/alif/alif/
            # List files changed in the PR relative to the merge-base with main.
            git diff --name-only origin/main...pr-${CHANGE_ID}
        ''',
        returnStdout: true
    ).trim()

    if (!changedFiles) {
        echo "No changed files detected -> running full build/test."
        return false
    }

    def files = changedFiles.readLines()
    echo "Changed files in PR #${env.CHANGE_ID}:"
    files.each { echo "  - ${it}" }

    def docsOnly = files.every { f ->
        f ==~ /(?i).*readme.*/ ||
        f ==~ /(?i).*\.(md|rst)$/ ||
        f.startsWith('doc/')
    }

    if (docsOnly) {
        echo " Only documentation/README files changed -> skipping build & test."
    } else {
        echo " Non-doc files changed -> running full build/test."
    }
    return docsOnly
}

def build_ble(String buildDir, String sample, String board, String conf_file = null) {
    echo "Sample    : ${sample}"
    echo "Board     : ${board}"
    echo "Build Dir : ${buildDir}"

    int rc = sh(
        script: """#!/bin/bash -x
            cd /root/alif/alif
            echo "Building ${sample} on ${board}"

            west build --force -p always -b ${board} \
                ${buildDir} --build-dir ${sample}

            BUILD_RC=\${PIPESTATUS[0]}
            if [ \$BUILD_RC -ne 0 ]; then
                echo "Build failed."
                exit \$BUILD_RC
            fi

            cp /root/alif/alif/${sample}/zephyr/zephyr.bin \$WORKSPACE/
            mv /root/alif/alif/${sample} \$WORKSPACE/
            cd \$WORKSPACE
            tar -vcf ${sample}.tar ${sample}
            exit 0
            """,
        returnStatus: true
    )

    if (rc == 0) {
        stash(name: "${sample}.bin", includes: "zephyr.bin")
        echo "✅ ${sample} succeeded"
        return true
    }

    echo "❌ ${sample} failed"
    return false
}

def flash_test(){
    sh """#!/bin/bash -xe
        pwd
        rsync -a --delete $ALIF_SETOOLS_ORIG $ALIF_SETOOLS_LOCATION
        cp zephyr.bin $ALIF_SETOOLS_LOCATION/build/images/zephyr.bin
        cp automation/B1-test-app.json $ALIF_SETOOLS_LOCATION/build/config/
        pushd $ALIF_SETOOLS_LOCATION
        sed -e 's/ttyUSB0/$SEDUT1/g' isp_config_data_temp.cfg > isp_config_data.cfg
        ./tools-config -p "B1 (AB1C1F4M51820PH0) - 1.8 MRAM / 2.0 SRAM" -r "A5"
        ./app-gen-toc --filename build/config/B1-test-app.json
        ./alif_hard_maintenance_mode_enable $RESET_TOOL -s /dev/$SEDUT1
        sleep 2
        ./app-write-mram -p -nr
        sed -e 's/ttyUSB0/$SEDUT2/g' isp_config_data_temp.cfg > isp_config_data.cfg
        ./alif_hard_maintenance_mode_enable $RESET_TOOL -s /dev/$SEDUT2
        sleep 2
        ./app-write-mram -p -nr
        pwd
        popd
        """
}

def test(String pytest_test){
    sh """#!/bin/bash -xe
        cd automation/pytest
        python3 -m venv venv
        . venv/bin/activate
        pip install -r requirements.txt
        sed -e 's/ttyACM0/$HEDUT1/g' -e 's/ttyACM1/$HEDUT2/g' pytest_ini.template > pytest.ini
        pytest $pytest_test --root-logdir=pytest-logs
        """
}

def get_all_alif_boards (){
    def output = sh(
        script: '''#!/bin/bash -xe
        cd /root/alif
        all_alif_boards=()
        while read -r name qualifiers; do
            IFS=',' read -ra qlist <<< "$qualifiers"
            for q in "${qlist[@]}"; do
                all_alif_boards+=("$name/$q")
            done
        done < <(west boards -n alif --format '{name} {qualifiers}')
        printf "%s\\n" "${all_alif_boards[@]}"
        ''',
        returnStdout: true
    ).trim()

    return output ? output.readLines() : []
}

def build_test_apps(boards, samples, args = null) {
    def stages       = [:]
    def allBoardList = (boards instanceof List)  ? boards  : [boards]
    def appList      = (samples instanceof List) ? samples : [samples]
    def boardArray   = allBoardList.join(' ')
    if (args != null) {
        args = (args instanceof List) ? args : [args]
    }

    appList.each { selectedAppEntry ->
        def appName = selectedAppEntry[0]
        def appPath = selectedAppEntry[1]

        stages[appName] = {
            catchError(buildResult: 'FAILURE', stageResult: 'FAILURE') {
                node('zas20') {
                    echo "Building Sample : ${appName}"
                    sh """#!/bin/bash -x
                    cd /root/alif/alif
                    # Calculate optimal build parallelism
                     CPU_COUNT=\$(nproc)
                    # Number of Jenkins executors configured on this node.
                    NODE_EXECUTORS=3
                    BUILD_THREADS=\$((CPU_COUNT / NODE_EXECUTORS))
                    if [ \$BUILD_THREADS -lt 2 ]; then
                        BUILD_THREADS=2
                    fi

                    echo "CPU_COUNT=[\$CPU_COUNT], BUILD_THREADS=[\$BUILD_THREADS]"
                    all_boards=(${boardArray})
                    overall_status=0
                    runCnt=0
                    skipCnt=0
                    failCnt=0
                    totalCnt=\${#all_boards[@]}

                    for boardName in "\${all_boards[@]}"; do
                        if [[ "\$boardName" == *apss* ]]; then
                            echo "Skipping \$boardName"
                            skipCnt=\$((skipCnt + 1))
                            continue
                        fi
                        boardDir="\${boardName//\\//_}"
                        build_dir="build_\${boardDir}_${appName}"
                        logfile="build_\${boardDir}_${appName}.log"
                        echo "🚩 Compiling for board: \$boardName, sample: ${appName} (dir: \$build_dir)"
                        echo ""
                        echo "=========================================="
                        echo "Board : \$boardName"
                        echo "Sample: ${appName}"
                        echo "Build : \$build_dir"
                        echo "Log   : \$logfile"
                        echo "=========================================="

                        west build --force -p always -b "\$boardName" ${appPath} --build-dir \$build_dir -DCMAKE_BUILD_PARALLEL_LEVEL=\$BUILD_THREADS
                        build_result=\$?

                        if [[ \$build_result -eq 0 ]]; then
                            echo "📌✅ Compilation succeeded for board: \$boardName, sample: ${appName}"
                            runCnt=\$((runCnt + 1))
                        else
                            echo "❌🚫 Build failed (code: \$build_result) for board: \$boardName, sample: ${appName}"
                            overall_status=1
                            failCnt=\$((failCnt + 1))
                        fi
                    done

                    echo "ℹ️ Run => Pass:\$runCnt/\$totalCnt, Fail: \$failCnt/\$totalCnt, Skip: \$skipCnt/\$totalCnt"
                    exit \$overall_status
                    """
                }
            }
        }
    }
    return stages
}

def checkManifestUpdate() {
    return sh(
        script: '''#!/bin/bash
            set -e
            #cd /root/alif/alif || exit 99

            last_commiter_email=$(git log -1 --format='%ae')
            automation_email=$(git config user.email)

            file_count=$(git diff-tree --no-commit-id --name-only -r HEAD | wc -l)
            changed_file_list=$(git diff-tree --no-commit-id --name-only -r HEAD)

            if printf '%s\n' "$changed_file_list" | grep -Fq "west.yml"; then
                is_west_updated=1
            else
                is_west_updated=0
            fi

            echo "Last committer email: $last_commiter_email"
            echo "Automation email: $automation_email"
            echo "No. of files changed: $file_count"

            if [ "$last_commiter_email" = "$automation_email" ] &&
               [ "$file_count" -eq 1 ] &&
               [ "$is_west_updated" -eq 1 ]; then
                echo "only west.yml file updation with automation"
                exit 1
            elif [ "$file_count" -eq 1 ] &&
                 [ "$is_west_updated" -eq 1 ]; then
                echo "only west.yml file updation manually"
                exit 2
            elif [ "$is_west_updated" -eq 1 ]; then
                echo "west.yml file updation found in last commit"
                exit 3
            else
                exit 0
            fi
        ''',
        returnStatus: true
    )
}

return this
