BOARD = 'alif_b1_dk/ab1c1f4m51820hh0/rtss_he'

def buildSample(String sampleName, String samplePath, String board = BOARD) {
    echo "DEBUG: buildSample(${sampleName}, ${samplePath}, ${board})"

    sh """
        set -eux
        echo "Building sample ${sampleName} from ${samplePath}"

        cd /root/alif/alif
        west forall -c "git clean -fdx"
        if [[ -v CHANGE_ID ]]; then
            git branch -D pr-${CHANGE_ID} || true
            git fetch origin pull/${CHANGE_ID}/head:pr-${CHANGE_ID}
            git checkout pr-${CHANGE_ID}
        else
            git fetch origin -pu
            git checkout main
            git reset --hard origin/main
        fi
        cd ..
        west update
        cd /root/alif/alif
        west build -p always -b ${board} ${samplePath}
        cp build/zephyr/zephyr-${sampleName}.bin ${WORKSPACE}
        tar -cvf Build-${sampleName}.tar build
    """

    stash name: "bin-${sampleName}", includes: "zephyr-${sampleName}.bin"
}
return this

