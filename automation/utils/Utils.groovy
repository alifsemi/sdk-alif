class Utils {

    def steps
    def github
    def git
    def config = [:]

    Utils(steps) {
        this.steps = steps
    }

    def init(Map cfg) {
        config = cfg
        def utilsPath = "${config.repo}/automation/utils"

        //steps.echo "Utils exists: ${steps.fileExists('sdk-alif/automation/utils/Utils.groovy')}"
        //steps.echo "GitHub exists: ${steps.fileExists("${utilsPath}/github/GitHubActions.groovy")}"
        //steps.echo "Git exists: ${steps.fileExists("${utilsPath}/git/GitActions.groovy")}"

        github = steps.load("${utilsPath}/github/GitHubActions.groovy")
        git    = steps.load("${utilsPath}/git/GitActions.groovy")

        if (github == null) { steps.error("Failed to load GitHubActions") }
        if (git == null) { steps.error("Failed to load GitActions") }
    }
}

return new Utils(this)
