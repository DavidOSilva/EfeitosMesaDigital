    .text

    .global  _dataMove

XMIT_BUFF_SIZE	.set 512;
_dataMove



  	add  #0, AR0  ; AR0 aponta para reception[N]
	add  #0, AR1  ; AR0 aponta para reception[N]

    rpt  #XMIT_BUFF_SIZE-1       ; Repetir para XMIT_BUFF_SIZE-2 elementos

    mov  *AR0+, *AR1+            ; Mover reception[i] para transmition[i] e incrementar os ponteiros

    ; Retornar da função
    ret

    .end
