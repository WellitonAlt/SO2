# SO2
Trabalho de SO2

Para compilar e adicionar ao kernel<br />
Use o Makefile <br />
sudo insmod arquivoGerado.ko<br /> <br />

Para testar teclado e ledGpio_2 <br />
echo "500" > /dev/ledBlink-0 <br />

Para encerar <br />
echo "-1" > /dev/ledBlink-0 <br />

Para remover do kernel <br />
sudo rmmod arquivoGerado <br />

