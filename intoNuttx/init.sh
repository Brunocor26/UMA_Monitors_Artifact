# init.sh - Arranca a malha de controlo da ventoinha de uma só vez.
#
# Ordem: primeiro os recetores (fan_app e servidor.wasm) e só depois o
# pulse_app, para os sockets UDP já estarem à escuta quando a telemetria
# começa a chegar.
#
# Correr no NSH com:  sh /mnt/sd0/init.sh
# (ou colocar como rcS de arranque)
#
# Ajusta os caminhos/portas/devices conforme o teu setup:
#   WASM   : /mnt/sd0/servidor.wasm
#   PWM    : /dev/pwm0   (atuador da ventoinha)
#   GPIO   : /dev/gpio14 (tacometro / contador de pulsos)
#   Portas : servidor escuta 5001, FanApp escuta 5002

# 1) Atuador PWM (recebe DUTY=, escuta em 5002)
fan_app -p 5002 -f 25000 /dev/pwm3 &
sleep 1

# 2) Controlador WASM (recebe RPM em 5001, envia DUTY p/ 127.0.0.1:5002)
#    --heap-size/--stack-size reduzem o consumo de RAM do iwasm (por defeito
#    sao ~128KB+64KB, o que esgota a memoria e impede o pulse_app de arrancar).
cd /mnt/sd0
sleep 1

iwasm --stack-size=24576 --addr-pool=0.0.0.0/0 servidor.wasm &
sleep 1

# 3) Sensor de RPM (le o GPIO, envia RPM p/ 127.0.0.1:5001)
pulse_app -t 1 -h 127.0.0.1 -p 5001 /dev/gpio14 &
