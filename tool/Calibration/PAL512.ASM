; -----------------------------------------------------------------------------
;	512色同時表示
; =============================================================================
;	2022/July/17th	Programmed by t.hara (HRA!)
; -----------------------------------------------------------------------------

vdp_p0	:= 0x98
vdp_p1	:= 0x99
vdp_p2	:= 0x9A
vdp_p3	:= 0x9B
msx_ver	:= 0x002d
ldirvm	:= 0x005c
bigfil	:= 0x016B
chgcpu	:= 0x0180
spr_gen	:= 0x7800
spr_col	:= 0x7400
spr_atr	:= 0x7600

		db		0xFE
		dw		start_address
		dw		end_address
		dw		start_address

		org		0xC000
start_address::
		;	スプライトを2つ透明で表示
		ld		hl, sprite_pattern
		ld		de, spr_gen
		ld		bc, 8
		call	ldirvm

		ld		hl, spr_col
		ld		bc, 512
		ld		a, 1
		call	bigfil

		ld		hl, sprite_attribute
		ld		de, spr_atr
		ld		bc, 12
		call	ldirvm

		;	turboR なら Z80モードに切り替える
		ld		a, [msx_ver]
		cp		a, 3
		jr		c, skip

		ld		a, 0x80			; Z80 with LED
		call	chgcpu

skip:
		di
		;	R#16 = 0
		xor		a, a
		out		[vdp_p1], a
		ld		a, 16 | 0x80
		out		[vdp_p1], a

		ld		c, vdp_p2

main_loop::
		in		a, [vdp_p1]
		ld		hl, palette0
		ld		e, 33

		;	スプライト衝突フラグ待ち
wait_sprite_collision:
		in		a, [vdp_p1]
		and		a, 0b00100000
		jp		z, wait_sprite_collision

palette_loop:
		;	Palette#X をパレットレジスタに書き込む
		repeat i,32
			outi
		endr
		;	WAIT
		ld		b, 55
wait_loop:
		djnz	wait_loop
		;	終了チェック
		dec		e
		jp		nz, palette_loop
		jp		main_loop

pal		macro	g, r
			repeat	b, 8
				db	(r << 4) | b, g
			endr
		endm

sprite_pattern::
		db		0xFF
		db		0
		db		0
		db		0
		db		0
		db		0
		db		0
		db		0

sprite_attribute::
		db		0
		db		0
		db		0
		db		0

		db		0
		db		0
		db		0
		db		0

		db		216
		db		0
		db		0
		db		0

palette0:
		pal		0, 0
		pal		1, 0
palette1:
		pal		2, 0
		pal		3, 0
palette2:
		pal		4, 0
		pal		5, 0
palette3:
		pal		6, 0
		pal		7, 0
palette4:
		pal		0, 1
		pal		1, 1
palette5:
		pal		2, 1
		pal		3, 1
palette6:
		pal		4, 1
		pal		5, 1
palette7:
		pal		6, 1
		pal		7, 1
palette8:
		pal		0, 2
		pal		1, 2
palette9:
		pal		2, 2
		pal		3, 2
palette10:
		pal		4, 2
		pal		5, 2
palette11:
		pal		6, 2
		pal		7, 2
palette12:
		pal		0, 3
		pal		1, 3
palette13:
		pal		2, 3
		pal		3, 3
palette14:
		pal		4, 3
		pal		5, 3
palette15:
		pal		6, 3
		pal		7, 3
palette16:
		pal		0, 4
		pal		1, 4
palette17:
		pal		2, 4
		pal		3, 4
palette18:
		pal		4, 4
		pal		5, 4
palette19:
		pal		6, 4
		pal		7, 4
palette20:
		pal		0, 5
		pal		1, 5
palette21:
		pal		2, 5
		pal		3, 5
palette22:
		pal		4, 5
		pal		5, 5
palette23:
		pal		6, 5
		pal		7, 5
palette24:
		pal		0, 6
		pal		1, 6
palette25:
		pal		2, 6
		pal		3, 6
palette26:
		pal		4, 6
		pal		5, 6
palette27:
		pal		6, 6
		pal		7, 6
palette28:
		pal		0, 7
		pal		1, 7
palette29:
		pal		2, 7
		pal		3, 7
palette30:
		pal		4, 7
		pal		5, 7
palette31:
		pal		6, 7
		pal		7, 7
palette32:
		space	32, 0
end_address::
