; Visual Studio ֧��masm���뷽����
; �Ҽ���Ŀ����-����������-�����Զ���-��ѡ��Masm��
; asm�ļ����Ҽ����ԣ�������ѡ��Microsoft Macro Assembler��

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