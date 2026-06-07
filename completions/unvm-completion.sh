_unvm_completions() {
  COMPREPLY=()

  local word options

  word="${COMP_WORDS[COMP_CWORD]}"

  if [[ ${COMP_CWORD} -le 1 ]]; then
    options="$(unvm complete --)"
  else
    options="$(unvm complete -- "${COMP_WORDS[@]:1:COMP_CWORD-1}")"
  fi

  COMPREPLY=( $(compgen -W "${options}" -- "${word}") )
}

complete -F _unvm_completions unvm
