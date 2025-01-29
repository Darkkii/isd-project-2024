.section .rodata
.global fs_index_start
fs_index_start:
.incbin "../data/index.html"
.global fs_index_end
fs_index_end:
.global fs_script_start
fs_script_start:
.incbin "../data/script.js"
.global fs_script_end
fs_script_end:
