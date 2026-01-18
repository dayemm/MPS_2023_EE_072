        PRESERVE8
        THUMB

;==================== DATA ====================
        AREA    MyData, DATA, READWRITE

; test data (bytes)
Marks   DCB 12,35,48,68,98,25,66,41,90,76,35,84,88,76,75,71,55,63,58,50,70
LEN     EQU 21

; output variables (word each)
CountGT70      SPACE 4
Count50to70    SPACE 4

;==================== CODE ====================
        AREA    |.text|, CODE, READONLY
        EXPORT  main
        ENTRY

; ------------ main ------------
main
        ; call: COUNT_GT70(ptr=len list, len)
        LDR     R0, =Marks
        MOV     R1, #LEN
        BL      COUNT_GT70
        ; R0 = count > 70
        LDR     R2, =CountGT70
        STR     R0, [R2]

        ; call: COUNT_50_70(ptr=len list, len)
        LDR     R0, =Marks
        MOV     R1, #LEN
        BL      COUNT_50_70
        ; R0 = count in [50..70]
        LDR     R2, =Count50to70
        STR     R0, [R2]

STOP    B       STOP               ; hang here for debugger

; ------------ COUNT_GT70 ------------
; R0 = pointer to array (bytes)
; R1 = length (elements)
; returns R0 = count of elements > 70
COUNT_GT70     PROC
        MOV     R3, #0             ; R3 = count
GT_Loop
        CMP     R1, #0
        BEQ     GT_Done
        LDRB    R2, [R0], #1       ; load byte and post-increment ptr
        CMP     R2, #70
        BHI     GT_Inc             ; > 70 ?
        ; not greater
GT_Next
        SUBS    R1, R1, #1
        BNE     GT_Loop
        B       GT_Done
GT_Inc
        ADD     R3, R3, #1
        B       GT_Next

GT_Done
        MOV     R0, R3             ; return value in R0
        BX      LR
        ENDP

; ------------ COUNT_50_70 ------------
; R0 = pointer to array (bytes)
; R1 = length (elements)
; returns R0 = count of elements 50..70 inclusive
COUNT_50_70    PROC
        MOV     R3, #0             ; R3 = count
RNG_Loop
        CMP     R1, #0
        BEQ     RNG_Done
        LDRB    R2, [R0], #1

        ; if R2 < 50 -> skip
        CMP     R2, #50
        BLO     RNG_Skip

        ; if R2 > 70 -> skip
        CMP     R2, #70
        BHI     RNG_Skip

        ; otherwise 50 <= R2 <= 70
        ADD     R3, R3, #1

RNG_Skip
        SUBS    R1, R1, #1
        BNE     RNG_Loop

RNG_Done
        MOV     R0, R3
        BX      LR
        ENDP

        END
