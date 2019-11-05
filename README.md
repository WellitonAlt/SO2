# SO2
Trabalho de SO2

Para compilar e adicionar ao kernel<br />
make <br />
sudo insmod teclado.ko <br /> <br />

Para testar <br />
echo "500" > /dev/ledBlink-0 <br />

Para encerar <br />
echo "-1" > /dev/ledBlink-0 <br />

Para remover do kernel <br />
sudo rmmod teclado <br />

