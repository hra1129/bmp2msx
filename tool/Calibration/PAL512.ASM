; -----------------------------------------------------------------------------
;	512色同時表示
; =============================================================================
;	2022/July/17th	Programmed by t.hara (HRA!)
; -----------------------------------------------------------------------------

vdp_p0	:= 0x98
vdp_p1	:= 0x99
vdp_p2	:= 0x9A
vdp_p3	:= 0x9B

		db		0xFE
		dw		start_address
		dw		end_address
		dw		start_address

		org		0xC000
start_address::
		di
		;	R#0 = 0b00010110
		ld		a, 0b00010110
		out		[vdp_p1], a
		ld		a, 0 | 0x80
		out		[vdp_p1], a

		;	R#16 = 0
		xor		a, a
		out		[vdp_p1], a
		ld		a, 16 | 0x80
		out		[vdp_p1], a

		ld		c, vdp_p2
main_loop::
		;	R#15 = 0 : ステータスレジスタ番号
		xor		a, a
		out		[vdp_p1], a
		ld		a, 15 | 0x80
		out		[vdp_p1], a

		;	V-SYNC待ち
wait_v_sync:
		in		a, [vdp_p1]
		and		a, 0x80
		jp		z, wait_v_sync

		ld		hl, palette0
		ld		de, (6 << 8) | 32

		;	R#15 = 1 : ステータスレジスタ番号
		ld		a, 1
		out		[vdp_p1], a
		ld		a, 15 | 0x80
		out		[vdp_p1], a
h_sync_loop:
		;	R#19 = 6*n : 走査線割り込み番号
		ld		a, d
		out		[vdp_p1], a
		add		a, 6
		ld		d, a
		ld		a, 19 | 0x80
		out		[vdp_p1], a
		;	Palette#X をパレットレジスタに書き込む
		ld		b, 32
		otir
		;	H-SYNC待ち
wait_h_sync:
		in		a, [vdp_p1]
		rrca
		jp		nc, wait_h_sync
		;	終了チェック
		dec		e
		jp		nz, h_sync_loop
		jp		main_loop

pal		macro	g, r
			repeat	b, 8
				db	(r << 4) | b, g
			endr
		endm

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

end_address::
