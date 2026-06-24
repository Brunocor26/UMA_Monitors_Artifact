# init.sh - Arranca a malha de controlo + supervisao da ventoinha de uma vez.
#
# Ordem: primeiro os atuadores/recetores (fan_app, led_app, servidor.wasm) e
# so depois o pulse_app, para os sockets UDP ja estarem a escuta quando a
# telemetria comeca a chegar.
#
# Correr no NSH com:  sh /mnt/sd0/init.sh
# (ou colocar como rcS de arranque)
#
# Ajusta os caminhos/portas/devices conforme o teu setup:
#   WASM   : /mnt/sd0/servidor.wasm
#   PWM    : /dev/pwm3   (atuador da ventoinha)
#   GPIO   : /dev/gpio14 (tacometro / contador de pulsos, entrada)
#   LED    : /dev/gpio21 (LED de alarme, saida; GP21)
#   Portas : servidor escuta 5001; FanApp 5002; LedApp 5003

# 1) Atuador PWM (recebe DUTY=, escuta em 5002)
fan_app -p 5002 -f 25000 /dev/pwm3 &
sleep 1

# 2) LED de alarme (recebe ALARM=, escuta em 5003; pisca enquanto ALARM=1)
led_app -p 5003 /dev/gpio21 &
sleep 1

# 3) Controlador + supervisao WASM
#    Recebe RPM em 5001; envia DUTY p/ :5002 e ALARM p/ :5003.
#    --stack-size reduz o consumo de RAM do iwasm (por defeito ~64KB).
cd /mnt/sd0
sleep 1

TARGET=${1:-600}
iwasm --stack-size=24576 --addr-pool=0.0.0.0/0 servidor.wasm $TARGET 5001 5002 127.0.0.1 5003 &
sleep 1

# 4) Sensor de RPM (le o GPIO, envia RPM p/ 127.0.0.1:5001)
pulse_app -t 1 -h 127.0.0.1 -p 5001 /dev/gpio14 &
