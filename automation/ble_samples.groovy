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
        fi
        cd ..
        west update
    '''
}

def verify_checkpatch(){
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
        fi
        ../zephyr/scripts/checkpatch.pl --ignore=GERRIT_CHANGE_ID,EMAIL_SUBJECT,COMMIT_MESSAGE,COMMIT_LOG_LONG_LINE -g pr-\${CHANGE_ID}...origin/main
            STATUS=\$?
        if [ \$STATUS -ne 0 ]; then
            exit \$STATUS
        else
        echo "Checkpatch passed successfully"
        fi
        '''
}

def verify_gitlint (){
    sh '''#!/bin/bash -xe
        env
        cd /root/alif/alif/
        pip install gitlint
        git log -$(git rev-list --count origin/main..HEAD) --pretty=%B | gitlint
        exit $?
        '''
}
def build_ble (String build_dir, String sample, String board, String conf_file=null){
    sh """#!/bin/bash -xe
        pwd
        echo "Sample: ${sample}"
        echo "Board: ${board}"
        echo "Build dir: ${build_dir}"
        cd /root/alif/alif/
        ls -la
        west build -p always -b ${board} ${build_dir} --build-dir ${sample}
        cp /root/alif/alif/${sample}/zephyr/zephyr.bin $WORKSPACE
        mv /root/alif/alif/${sample} $WORKSPACE
        cd $WORKSPACE
        tar -cvf ${sample}.tar ${sample}/
        pwd
        """
        stash name: "${sample}.bin", includes: 'zephyr.bin'

}



return [
    initialize: this.&initialize,
    verify_checkpatch: this.&verify_checkpatch,
    verify_gitlint: this.&verify_gitlint,
    build_ble: this.&build_ble
]