SECTION code_user

PUBLIC _print_string

;----------
; print_string
; inputs: hl = first position of a null ($00) terminated string
;----------
_print_string:
            ld   a, (hl) ; a = character to be printed
            or   a ; sets z register if 0
            ret  z ; return if z register set
            rst  $10 ; prints the character
            inc  hl ; hl = next character
            jr   _print_string ; loop