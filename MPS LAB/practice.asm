; Marks   DCB 12,35,48,68,98,25,66,41,90,76,35,84,88,76,75,71,55,63,58,50,70
; GT70 4
; RANGE50_70 4
; N 21
        AREA    MyData, DATA, READWRITE   ; define a writable data section

Marks   DCB     12,35,48,68,98,25,66,41,90,76,35,84,88,76,75,71,55,63,58,50,70
        ALIGN                               ; align to next 4-byte boundary

GT70    DCD     0                           ; counter for >70
RANGE50_70
        DCD     0                           ; counter for 50–70 inclusive

N       EQU     21                          ; total number of marks (constant)
