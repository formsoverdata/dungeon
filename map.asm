MAP_SIZE: equ $10 ; 16

SECTION code_user

PUBLIC _get_map_tile
PUBLIC _set_map_tile

;----------
; _get_map_tile
; inputs: b = x, c = y
; outputs: hl = cell location within grid
; alters: a, b, de, hl
;----------
_get_map_tile:
            pop hl ; hl = ret address
            pop bc ; b = x, c = y
            push hl ; ret address back on stack
            call load_cell_location
            ld a, (hl) ; load 8 bit value into a
            ld h, $00
            ld l, a ; hl = tile
            ret

;----------
; _set_map_tile:
; inputs: b = x, c = y, e = tile
; alters: a, b, de, hl
;----------
_set_map_tile:
            pop hl ; hl = ret address
            pop bc ; b = x, c = y
            pop de
            push hl ; ret address back on stack
            push de ; store e=tile
            call load_cell_location
            pop de ; restore e=tile
            ld (hl), e ; load 8 bit value into location
            ret

;----------
; load_cell_location
; inputs: b = x, c = y
; outputs: hl = cell location within grid
; alters: a, b, de, hl
;----------
load_cell_location:            
            ld a, b ; load a with x
            ld hl, _map ; point hl at _map
            ld d, $00 
            ld e, a ; de = a
            add hl, de ; hl = _map + x
            ;ld d, $00 - already 0
            ld e, c ; de = y           
            ex de, hl ; hl = y
            add hl, hl
            add hl, hl
            add hl, hl
            ;add hl, hl
            ;add hl, hl
            add hl, hl; hl = y * MAP_WIDTH(=64)
            add hl, de ; hl = _map + x + (y * MAP_WIDTH)
            ret

SECTION bss_user

_map: ds MAP_SIZE*MAP_SIZE