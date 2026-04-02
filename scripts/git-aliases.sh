#!/usr/bin/env bash

alias gs='git status --short'
alias ga='git add'
alias gd='git diff'
alias gdc='git diff --cached'
alias gl='git log --oneline --decorate --graph -n 15'
alias gb='git branch'
alias gbv='git branch -vv'
alias gsw='git switch'
alias gswc='git switch -c'
alias gcm='git commit -m'
alias gp='git push'
alias menu='print_git_aliases'

print_git_aliases() {
    cat <<'EOF'

========================================
Git aliases
========================================
  ga     git add
  gs     git status --short
  gd     git diff
  gdc    git diff --cached
  gl     git log --oneline --decorate --graph -n 15
  gb     git branch
  gbv    git branch -vv
  gsw    git switch
  gswc   git switch -c
  gcm    git commit -m "<message>"
  gp     git push
  menu   show this menu
========================================

EOF
}
