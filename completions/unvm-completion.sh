_unvm_completions() {
  COMPREPLY=()

  local word commands flags list_flags use_flags versions

  word="${COMP_WORDS[COMP_CWORD]}"

  commands="install i remove r use u list l ls la"
  flags="? -? -h --help"
  list_flags="-a --available"
  use_flags="-l --local"
  versions="latest lts none"

  if [[ ${COMP_CWORD} -eq 1 ]]; then
    COMPREPLY=( $(compgen -W "${flags} ${commands}" -- ${word}) )
    return 0
  fi

  case "${COMP_WORDS[1]}" in
    install|i|remove|r)
      COMPREPLY=( $(compgen -W "${flags} ${versions}" -- ${word}) )
      ;;
    use|u)
      COMPREPLY=( $(compgen -W "${flags} ${use_flags} ${versions}" -- ${word}) )
      ;;
    list|l)
      COMPREPLY=( $(compgen -W "${flags} ${list_flags}" -- ${word}) )
      ;;
    *)
      COMPREPLY=( $(compgen -W "${flags}" -- ${word}) )
      ;;
  esac
}

complete -F _unvm_completions unvm
