#  Copyright Cypress Semiconductor Corporation, 2010-2011,
#  All Rights Reserved
#  UNPUBLISHED, LICENSED SOFTWARE.
#
#  CONFIDENTIAL AND PROPRIETARY INFORMATION
#  WHICH IS THE PROPERTY OF CYPRESS.
#
#  Use of this file is governed
#  by the license agreement included in the file
#
#     <install>/license/license.txt
#
#  where <install> is the Cypress software
#  installation root directory path.
#

# Cypress FX3 Firmware Startup code


.section .text
.code 32

.global jump
jump:
    bx  R0

.global CyU3PToolChainInit
CyU3PToolChainInit:

# clear the BSS area
__main:
	mov	R0, #0
	ldr	R1, =_bss_start
	ldr	R2, =_bss_end
1:	cmp	R1, R2
	strlo	R0, [R1], #4
	blo	1b

	b	main


.global __user_initial_stackheap
__user_initial_stackheap:

# The tool chain is not expected to place the stack.
# No heap is expected to be used by USB 3.0 platform drivers.
# Place them as required by the user code
.if  INTER == TRUE
    bx      lr                 
.else
    mov     pc, lr             
.endif

.end

# []
