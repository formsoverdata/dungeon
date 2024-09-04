; used to populate attribute memory from buffer via _copy_attr_buffer
VIDEOATT: equ $5800 ; address of attribute RAM
VIDEOATT_L: equ $0300 ; length of attribute RAM
ATTR_BUFF: equ $F800 ; hard coded attribute buffer address
; used to populate only the 2nd half of attribute memory via _copy_attr_buffer_half
VIDEOATT_HALF: equ $5980 ; address of 2nd half or attribute RAM
VIDEOATT_L_HALF: equ $0180 ; half length of attribute RAM
ATTR_BUFF_HALF: equ $F980 ; hard coded attribute buffer 2nd half

SECTION code_user

PUBLIC _fill_rectangle_char
PUBLIC _fill_rectangle_attr
PUBLIC _copy_attr_buffer
PUBLIC _copy_attr_buffer_half

;----------
; _fill_rectangle_char
; inputs: d = y, e = x, h = width, l = height, ix = address of first char (accepts strings and repeats full string)
;----------
_fill_rectangle_char:
            pop bc ; bc = ret address
            pop de ; d = y, e = x
            pop hl ; h = width, l = height
            pop ix ; ix = address of char
            push bc ; ret address back on stack
            ld iy, ix
            ld b, l ; set counter 1 to height
            ld c, d ; store initial y in c
_fill_rectangle_char_loop1:
            ld d, c ; retrieve initial y
            push hl ; store width/height
            push bc ; store counter 1
            ld b, h ; set counter 2 to width
            ;----------
            ; BEGIN - get_char_address (inline) - adapted from a routine by Dean Belfield
            ; inputs: d = y, e = x
            ; outputs: hl = location of screen address
            ;----------
            ld a,e
            and $07
            rra
            rra
            rra
            rra
            or d
            ld l,a
            ld a,e
            and $18
            or $40
            ld h,a
            ; END
_fill_rectangle_char_loop2:                                    
            push hl ; store hl = screen address
            push bc ; store counter 2
            push de ; store x and y
            ld a, (ix)
            or a ; compare with null (equiv. cp $00)
            jr nz, _fill_rectangle_char_print_char ; if not null goto print            
            ld ix, iy 
            ld a, (ix) ; reset string 
_fill_rectangle_char_print_char:
            ld de, udgs
            add a, a
            add a, a
            add a, a
            add de, a
            ld b, 8 ; loop counter
_fill_rectangle_char_loop3:
            ld a, (de) ; get the byte
            ld (hl), a ; print to screen
            inc de ; goto next byte of character
            inc h ; goto next line of screen
            djnz _fill_rectangle_char_loop3 ; loop 8 times
            pop de ; retrieve x and y
            pop bc ; retrieve counter 2
            pop hl ; retrieve hl = screen adresss
            inc hl ; goto next character
            inc d ; increase y
            inc ix ; increase char
            djnz _fill_rectangle_char_loop2                         
            pop bc ; retrieve counter 1
            pop hl ; retrieve width/height
            inc e ; increase x
            djnz _fill_rectangle_char_loop1  
            ret       

;----------
; _fill_rectangle_attr
; inputs: d = y, e = x, h = width, l = height, i = paper, x = ink
;----------
_fill_rectangle_attr:
            pop bc ; ix = ret address
            pop de ; d = y, e = x
            pop hl ; h = width, l = height
            pop ix ; i = paper, x = ink
            push bc ; ret address back on stack
            ld b, l ; set counter 1 to height
            ld c, d ; store initial y in c
_fill_rectangle_attr_loop1:
            ld d, c ; retrieve initial y
            push hl ; store width/height
            push bc ; store counter 1
            ld b, h ; set counter 2 to width                        
            push bc ; store counter 2
            ;----------
            ; BEGIN - get_attr_address (inline) - adapted from a routine by Jonathan Cauldwell
            ; inputs: d = y, e = x
            ; outputs: hl = location of attribute (buffer) address
            ;----------
            ld bc, ATTR_BUFF
            ld a,e
            rrca
            rrca
            rrca
            ld l,a
            and $03            
            add a, b ; add hi byte of ATT_BUFF address
            ld h,a
            ld a,l
            and $e0
            ld l,a
            ld a,d
            add a, l
            ld l,a
            ; END
            pop bc ; retrieve counter 2
_fill_rectangle_attr_loop2:
            push bc ; store counter 2
            ld bc, ix ; retrieve paper/ink
            ld a, b ; load paper
            rla 
            rla
            rla ; paper in position
            or c ; add ink
            ld (hl), a ; store in location
            inc hl ; go to next location
            pop bc ; retrieve counter 2
            inc d ; increase y
            djnz _fill_rectangle_attr_loop2                         
            pop bc ; retrieve counter 1
            pop hl ; retrieve width/height
            inc e ; increase x
            djnz _fill_rectangle_attr_loop1  
            ret

;----------
; copy_attr_buffer
; copy attribute buffer into attribute memory
; alters: hl, de, bc
;----------
_copy_attr_buffer:
            ld de, VIDEOATT ; target is attribute memory
            ld hl, ATTR_BUFF ; source is attribute buffer
            ld bc, VIDEOATT_L ; length is size of attribute memory
            ldir ; copy
            ret

;----------
; copy_attr_buffer_half
; copy 2nd half of attribute buffer into 2nd half of attribute memory (opt)
; alters: hl, de, bc
;----------
_copy_attr_buffer_half:
            ld de, VIDEOATT_HALF ; target is attribute memory
            ld hl, ATTR_BUFF_HALF ; source is attribute buffer
            ld bc, VIDEOATT_L_HALF ; length is size of attribute memory
            ldir ; copy
            ret

SECTION rodata_user
PUBLIC _background_pattern1
PUBLIC _background_pattern2
_background_pattern1: ; example of how we could build up a repeating background
defb "@ABCDEFGHIJKLMNOPQRSTUVWZYX[", $00
_background_pattern2:
defb "QWOINEIOUWEQPOKCEUIBWECXIUY@", $00
udgs: ; currently just a font, but can use for graphics
defb 0,0,0,0,0,0,0,0 ; @ - space
defb 126,129,189,189,129,189,165,231 ; A
defb 254,131,189,131,189,189,129,254 ; B
defb 126,195,189,167,167,189,195,126 ; C
defb 252,134,187,165,165,187,134,252 ; D
defb 126,195,189,135,191,189,195,126 ; E
defb 255,129,191,130,190,160,160,224 ; F
defb 126,195,189,191,177,189,195,126 ; G
defb 231,165,189,129,189,165,165,231 ; H
defb 127,65,119,20,20,119,65,127 ; I
defb 7,5,5,229,165,189,195,126 ; J
defb 238,186,182,140,182,187,173,231 ; K
defb 224,160,160,160,160,191,129,255 ; L
defb 231,189,153,165,189,165,165,231 ; M
defb 231,181,157,173,181,185,173,231 ; N
defb 126,129,189,165,165,189,129,126 ; O
defb 254,131,189,189,131,190,160,224 ; P
defb 126,195,189,189,173,181,195,126 ; Q
defb 254,131,189,189,129,187,173,231 ; R
defb 126,194,190,195,253,189,195,126 ; S
defb 255,129,239,40,40,40,40,56 ; T
defb 231,165,165,165,165,189,129,126 ; U
defb 231,165,165,165,189,219,102,60 ; V
defb 231,165,165,165,189,165,219,126 ; W
defb 231,189,219,102,102,219,189,231 ; X
defb 199,109,187,214,108,40,40,56 ; Y
defb 255,129,251,54,108,223,129,255 ; Z
defb 255,255,255,255,0,0,0,0 ; [ - horizontal stripe
defb %11110000
defb %11110000
defb %11110000
defb %11110000
defb %11110000
defb %11110000
defb %11110000
defb %11110000 ; \ - vertical stripe
