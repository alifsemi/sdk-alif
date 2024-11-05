List of documents
=================

|Folder   | Document Name                               |
|---------|---------------------------------------------|
|augd00xx | Getting Started With Zephyr Alif SDK        |

Tested with Ubuntu 22.04. The goal is to build the documentation in html- and pdf-format.

Prerequisites
=============
To be able to build the documentation a number of packages needs to be installed.
It's assumed you have installed all the documentation tools required by the Zephyr RTOS itself.
```
pip install -r zephyr/scripts/requirements-doc.txt
```

For Alif specific requirements please run the following command on the SDK top-level scripts-folder
```
pip install -r alif/scripts/requirements-doc.txt
```

For PDF output install:
```
sudo apt install texlive-full
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
