; Visual Studio 支持masm编译方法：
; 右键项目属性-生成依赖项-生成自定义-勾选“Masm”
; asm文件，右键属性，项类型选“Microsoft Macro Assembler”

ifndef _CL_ARCH_X64
.686p
.XMM
.model flat, C
endif

.code  
_cl_Break proc
	int 3
	ret
_cl_Break ENDP 

_cl_NoOperation proc
  nop
  ret
_cl_NoOperation endp

barrier proc
  nop
  nop
  nop
  nop
  ret
barrier endp
end