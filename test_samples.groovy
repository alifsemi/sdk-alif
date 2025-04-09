def clean_zephyr_cache() {
    sh """#!/bin/bash -xe
        rm -f /media/share/jenkins_share/zephyrproject_sdk_alif.tar.gz"""
}

def zephyr_init() {
    sh """#!/bin/bash -xe
        env
        printenv NODE_NAME
        eval `ssh-agent -s`
        rm -rf venv
        mkdir venv
        python3 -m venv venv
        . venv/bin/activate
        pip install wheel
        pip install west
        deactivate"""

    if (fileExists('/media/share/jenkins_share/zephyrproject_sdk_alif.tar.gz')) {
        sh """#!/bin/bash -xe
            eval `ssh-agent -s`
            . venv/bin/activate
            cp /media/share/jenkins_share/zephyrproject_sdk_alif.tar.gz .
            tar xf zephyrproject_sdk_alif.tar.gz
            cd zephyrproject/alif
            git status
            git clean -fd
            git pull
            if [[ -v CHANGE_ID ]]; then
              git branch -D pr-\${CHANGE_ID} || true
              git clean -fd
              git fetch origin pull/\${CHANGE_ID}/head:pr-\${CHANGE_ID}
              git checkout pr-\${CHANGE_ID}
            fi
            west update
            west zephyr-export
            pip install -r scripts/requirements.txt
            pip install -r ../zephyr/scripts/requirements.txt
            cd ../..
            deactivate"""
    } else {
        sh """#!/bin/bash -xe
            . venv/bin/activate
            eval `ssh-agent -s`
            west init zephyrproject -m git@github.com:alifsemi/sdk-alif.git
            cd zephyrproject/alif
            west update
            west zephyr-export
            pip install -r scripts/requirements.txt
            pip install -r ../zephyr/scripts/requirements.txt
            cd ../..
            tar -czvf zephyrproject_sdk_alif.tar.gz zephyrproject
            cp zephyrproject_sdk_alif.tar.gz /media/share/jenkins_share/
            cd zephyrproject/
            cd alif
            if [[ -v CHANGE_ID ]]; then
              git branch -D pr-\${CHANGE_ID} || true
              git clean -fd
              git fetch origin pull/\${CHANGE_ID}/head:pr-\${CHANGE_ID}
              git checkout pr-\${CHANGE_ID}
            fi
            west update
            cd ../..
            deactivate"""
    }
}

def verify_checkpatch() {
    sh """#!/bin/bash -x
        set +e
        env
        . venv/bin/activate
        cd zephyrproject/alif
        git fetch --all
        if [[ -v CHANGE_ID ]]; then
          GIT_REMOTE=`git remote`
          echo \$GIT_REMOTE
          if ../zephyr/scripts/checkpatch.pl --ignore=GERRIT_CHANGE_ID,EMAIL_SUBJECT,COMMIT_MESSAGE,COMMIT_LOG_LONG_LINE -g pr-\${CHANGE_ID}...\${GIT_REMOTE}/main ; then
            echo "Checkpatch OK"
          else
            echo "ERROR: ../zephyr/scripts/checkpatch.pl detected errors/warnings."
          fi
        else
          echo "Checkpatch OK - skipped"
        fi

        deactivate"""
}

def build_zephyr(String sample, String build_dir, String board, String conf_file=null) {
    //if (!sample or !build_dir or !board) {
    //	error "Failed, null arguments given... No reason to continue execution"
    //}
    println(sample)
    println(build_dir)
    println(board)
    println(conf_file)
    String overlay = ""
    if (conf_file) {
    	overlay = "-- -DOVERLAY_CONFIG=$conf_file"
    }
    sh """#!/bin/bash -x
        set +e
        env
        . venv/bin/activate
        cd zephyrproject/zephyr
        west build -p always -b $board --build-dir $build_dir $sample $overlay
        deactivate"""
        // def proc = "echo \$?".execute()
        // if [ proc.exitValue() -eq 0 ]; then
        //   echo "Build succeeded"
        // else
        //   echo "Build failed"
        // fi
        //manual fail
        //error "Failed, Maintenance Mode not Enabled... No reason to continue execution"
}

def run_test(String jsonfile, String zephyr_build_dir, String test) {

    sh """#!/bin/bash -xe
        printenv NODE_NAME
        sed -i "s/ttyUSB.*/ttyAlifSEDUT1/g" $ALIF_SETOOLS_ORIG/isp_config_data.cfg
        rsync -a --delete $ALIF_SETOOLS_ORIG $ALIF_SETOOLS_LOCATION
        cp $zephyr_build_dir/zephyr.bin $ALIF_SETOOLS_LOCATION/build/images/
        cp $jsonfile $ALIF_SETOOLS_LOCATION/build/config/
        pushd $ALIF_SETOOLS_LOCATION/
        ./app-gen-toc --filename build/config/$jsonfile
        ./app-write-mram -p
        popd"""

    sh """#!/bin/bash -xe
        pushd pytest
        export PATH=$PATH:$JLINK
        sed -e "s/ttyUSB2/ttyAlifSE$DUT_NUMBER/g" -e "s/ttyUSB1/ttyAlifUART2$DUT_NUMBER/g" -e "s/ttyUSB0/ttyAlifUART4$DUT_NUMBER/g" pytest.ini.template > pytest.ini
        cat pytest.ini
        mkdir junit
        mkdir pytest-logs
        mkdir venv
        python3 -m venv venv
        . venv/bin/activate
        pip install -r requirements.txt
        pytest -s -k $test --tb=native --junit-xml junit/zephyr-junit-report.xml --root-logdir=pytest-logs
        deactivate
        popd
        """
}


return [
    build_zephyr: this.&build_zephyr,
    run_test: this.&run_test,
    clean_zephyr_cache: this.&clean_zephyr_cache,
    zephyr_init: this.&zephyr_init,
    verify_checkpatch: this.&verify_checkpatch
]
