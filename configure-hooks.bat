@echo off

IF "%~1"=="" GOTO Usage

IF "%~1"=="install" (
    pre-commit install --install-hooks
    pre-commit install --install-hooks -t commit-msg
    git config commit.template COMMIT_MESSAGE_TEMPLATE
    GOTO End
)

IF "%~1"=="uninstall" (
    pre-commit uninstall
    pre-commit uninstall -t commit-msg
    git config --unset commit.template
    GOTO End
)

:Usage
ECHO Usage: %0 {install^|uninstall}
GOTO End

:End
