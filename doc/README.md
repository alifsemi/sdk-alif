List of documents
=================

| Folder         | Document Name               | Formats   | Description                            |
|----------------|-----------------------------|-----------|----------------------------------------|
| sdk            | SDK Technical Documentation | HTML      | SDK Technical documentation            |
| user_guide     | User Guide                  | HTML, PDF | Getting started guide                  |
| release_notes  | Release Notes               | HTML, PDF | Release information                    |
| appnotes       | Application Notes           | HTML, PDF | Detailed guides for specific use cases |

Each document is built separately.

Prerequisites
=============
To be able to build the documentation a number of packages needs to be installed.
It's assumed you have installed all the documentation tools required by the Zephyr RTOS itself.
```
pip install -r zephyr/doc/requirements.txt
```

For Alif specific requirements please run the following command on the SDK top-level scripts-folder
```
pip install -r alif/scripts/requirements-doc.txt
```

For PDF output install:
```
sudo apt-get install texlive-latex-recommended texlive-fonts-extra texlive-fonts-recommended texlive-latex-extra latexmk
```

Building
========

HTML
----
Either
```
cd ONE_OF_THE_DOC_FOLDERS
make html
```
or
```
cd ONE_OF_THE_DOC_FOLDERS
make singlehtml
```

PDF
===
```
cd ONE_OF_THE_DOC_FOLDERS
make latexpdf
```

