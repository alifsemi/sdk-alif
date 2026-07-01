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
     sh '''#!/bin/bash -xe
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
        '''
}

def verify_gitlint (){
    sh '''#!/bin/bash -xe
        env
        cd /root/alif/alif/
        if [[ -v CHANGE_ID ]]; then
            pip install gitlint
            git rev-list origin/main..HEAD | xargs -r -n1 gitlint --commit
        fi
        exit $?
        '''
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
                            echo "📌 ✅ Compilation succeeded for board: \$boardName, sample: ${appName}"
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

return [
    initialize: this.&initialize,
    verify_checkpatch: this.&verify_checkpatch,
    verify_gitlint: this.&verify_gitlint,
    build_ble: this.&build_ble,
    flash_test: this.&flash_test,
    test: this.&test,
    get_all_alif_boards: this.&get_all_alif_boards,
    build_test_apps: this.&build_test_apps
]
