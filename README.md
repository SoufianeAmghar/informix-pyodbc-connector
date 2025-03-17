## informix-pyodbc-connector
This project will help developers access the IBM informix database using Python and Pyodbc as a connector

## Getting Started

### Prerequisites Installation

First, you will need to install the followings:

1. Python 3.7 or above
2. Pip

## DRIVER setup

### This setup is only for non Linux x64 users :
1- If you don't have a Linux x64 distribution, go to the following page to download and install the driver for your operating system along with it's requirements :
(https://www.progress.com/odbc/ibm-informix)
2- Follow their documentation and then update the .env INFORMIX_DRIVER with your directory

### For Linux users, make sure you run the following commands :

1- Install the different requirements for the driver to start from this website :
(https://docs.progress.com/bundle/datadirect-odbc-readmes/page/topics/odbc/informix-odbc-8.0.2-unix.html)

2- Identify which Shell you are using 

```sh
echo $SHELL
```
Common shells are: (Bash- /bin/bash & Zsh- /bin/zsh)

3. Add LD_LIBRARY_PATH to the Shell Configuration File after you have your driver inside the api folder. 
Change the command based on your absolute path and number of bits (32/64) of your machine

For Bash:

```sh
export LD_LIBRARY_PATH=YOUR_ABS_PATH/api/linux_BITS_driver_lib/ODBC_BITSbit/lib:$LD_LIBRARY_PATH
source ~/.bashrc
```

For Zsh:

```sh
export LD_LIBRARY_PATH=YOUR_ABS_PATH/api/linux_BITS_driver_lib/ODBC_BITSbit/lib::$LD_LIBRARY_PATH
source ~/.zshrc
```


4. Verify the Configuration

After adding the export command, verify that LD_LIBRARY_PATH is set correctly:

```sh
echo $LD_LIBRARY_PATH
```

## API setup

1- Go inside the api directory of the project

2- Run the following commands if you don't have Poetry :

```sh
pip install poetry
poetry --version
```
3- Install the project requirements, create a virtual environment and start the application :

```sh
poetry config virtualenvs.in-project true
poetry install
poetry shell
python main.py
```

