# SO2
Trabalho de SO2

Para compilar e adicionar ao kernel
make
sudo insmod teclado.ko


Para testar
echo "500" > /dev/ledBlink-0
  -> "500" frequencia

Para encerar
echo "-1" > /dev/ledBlink-0


Para remover do kernel
sudo rmmod teclado

