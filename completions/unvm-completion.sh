_unvm_completions() {
  COMPREPLY=()

  local word commands versions

  word="${COMP_WORDS[COMP_CWORD]}"

  commands="install i remove r use u list l ls la"
  versions="latest lts none"

  if [[ ${COMP_CWORD} -eq 1 ]]; then
    COMPREPLY=( $(compgen -W "${commands}" -- ${word}) )
    return 0
  fi

  case "${COMP_WORDS[1]}" in
    install|i|remove|r|use|u)
      COMPREPLY=( $(compgen -W "${versions}" -- ${word}) )
      ;;
    list|l)
      if [[ ${COMP_CWORD} -eq 2 ]]; then
      COMPREPLY=( $(compgen -W "available" -- ${word}) )
      fi
      ;;
    *)
      COMPREPLY=()
      ;;
  esac
}

complete -F _unvm_completions unvm
