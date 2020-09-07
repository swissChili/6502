require "blackjack"

site = Site

-- site.processors.md = markdownProcessor
site.processors.scss = {
	process = cmdProcessor("sass"),
	extension = "css"
}
site.processors.webm = {
	process = cmdProcessor("cat"),
	extension = "webm"
}

site:build()
