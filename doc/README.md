# Documentation

| Folder         | Document                    | Formats   | Description                            |
|----------------|-----------------------------|-----------|----------------------------------------|
| user_guide     | User Guide                  | HTML, PDF | Getting started guide                  |
| appnotes       | Application Notes           | HTML, PDF | Detailed guides for specific use cases |
| release_notes  | Release Notes               | HTML, PDF | Release information                    |
| sdk            | SDK Technical Documentation | HTML      | SDK technical documentation            |

Each document is built separately. These steps are for a new Ubuntu 22.04 or 24.04 machine. They use a direct clone of this repository. After `west init`, the same files live under `alif/`; run the commands from that directory.

## 1. Install system packages

```bash
sudo apt update
sudo apt install -y git python3 python3-venv python3-pip make
```

## 2. Get the repository

```bash
git clone https://github.com/alifsemi/sdk-alif.git
cd sdk-alif
```

For a tested release, check out a tag instead of `main`, for example `git checkout v2.4.0`.

## 3. Install the documentation tools

Ubuntu does not allow `pip` to install into the system Python. Use a virtual environment, and activate it again in every new terminal before building.

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install -U pip
pip install -r scripts/requirements-doc.txt
pip install 'sphinx-copybutton>=0.5.2'
```

`sphinx-copybutton` provides the `sphinx_copybutton` extension used by every document. Without it, `make html` stops with `No module named 'sphinx_copybutton'`. If that venv already exists, activate it and run the last `pip install` line, then run `make html` again.

## 4. Build the HTML

`make html` must be run inside one document folder. It runs `sphinx-build` and writes `build/html/index.html` in that folder.

```bash
cd doc/appnotes
make html
cd ../..

cd doc/user_guide
make html
cd ../..

cd doc/release_notes
make html
cd ../..

cd doc/sdk
make html
```

`make singlehtml` builds one page instead of a page per chapter.

## 5. Open it in a browser

From the document folder you just built:

```bash
xdg-open build/html/index.html
```

Or open the file directly, for example `doc/appnotes/build/html/index.html`.

Run `make html` again from the same folder after editing the sources. `make clean` deletes the generated pages.

## PDF

HTML does not need LaTeX. To build a PDF, install LaTeX and run `make latexpdf` from `doc/user_guide`, `doc/appnotes`, or `doc/release_notes`:

```bash
sudo apt install texlive-latex-recommended texlive-fonts-extra texlive-fonts-recommended texlive-latex-extra latexmk
make latexpdf
```
