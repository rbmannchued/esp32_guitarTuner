// codigo baseado e adaptado de Jonatan Zientarski > https://www.youtube.com/@jonatanrrz


#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <arduinoFFT.h>
#include <Wire.h>


unsigned int amostras = 1024;
volatile int indice_amostrar = 1024;
double amplitude_pico;
double freq_pico;
double dados_real[1024], dados_imag[1024];



Adafruit_SSD1306 display = Adafruit_SSD1306();
ArduinoFFT<double> FFT = ArduinoFFT<double>(dados_real,dados_imag,amostras,1024);



hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
void IRAM_ATTR onTimer(){
  portENTER_CRITICAL_ISR(&timerMux);
  if(indice_amostrar < amostras){
    dados_real[indice_amostrar] = analogRead(35);
    dados_imag[indice_amostrar] = 0;
    indice_amostrar++;
  }
  portEXIT_CRITICAL_ISR(&timerMux); // sai da interrupção

}



void setup() {
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(1);
  
  timer = timerBegin(20000000);  
  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, 19531, true,0);
 // timerAlarmEnable(timer);
  tone(18, 110);

  
}

void loop() {
  afinador();
  amostra_calcula_FFT();
}
void amostra_calcula_FFT(){
  indice_amostrar = 0;
  amplitude_pico = 0;
  while(indice_amostrar < amostras);
  FFT.dcRemoval();//remove o offset
  FFT.compute(FFTDirection::Forward);
  FFT.complexToMagnitude();
  for(int i = 2; i < (amostras/2); i++){
    if(dados_real[i] > amplitude_pico){
      freq_pico = i;
      amplitude_pico = dados_real[i];
    }

  }
}
void afinador(){
  float freq_central, freq_max, freq_min, largura_faixa;
  char notacao[2];
  if(freq_pico<65){
    notacao[0] = '-'; notacao[1] = '-';
    freq_min = 0;
    freq_central =0;
    freq_max = 0;
    largura_faixa = 1;
  }
  if(freq_pico>75 && freq_pico<=79){
    notacao[0] = 'D'; notacao[1] = '2';
    freq_min =75;
    freq_central =77;
    freq_max = 79;
    largura_faixa = 4;
  }

  if(freq_pico>80 && freq_pico<=84){
    notacao[0] = 'E'; notacao[1] = '2';
    freq_min =80;
    freq_central =82;
    freq_max = 84;
    largura_faixa = 4;
  }

  display.clearDisplay();
  display.drawLine(0,0,0,63,1);
  display.drawLine(0,0,82,0,1);
  display.drawLine(82,0,82,63,1);
  display.drawLine(0,63,82,63,1);

  if(freq_pico<100)display.setCursor(96,9);
  else display.setCursor(90,9);

  display.setTextSize(2);
  display.print(freq_pico, 0);
  display.setCursor(97,45);
  display.print(notacao);
  display.setTextSize(1);
  display.print("Hz");

  for(float i = -0.7; i <= 0.7;i = i + 0.015){
    display.drawPixel(41+sin(i)*50,62-cos(i)*50,1);
  }
    display.drawLine(1,61,81,61,0); //corrigir bugs visuais
    display.drawLine(1,62,81,62,0);

    float horizontal = (freq_pico - freq_central)*(66/largura_faixa)+41;
    if(freq_central >= 68){
      display.drawLine((int)horizontal,17+abs((horizontal=41)*0.36),41,60,1);
    }else{
      display.setTextSize(2);
      display.setCursor(36,32);
      display.print("?"); 
    }

    display.drawCircle(41,59,3,1);
    display.drawRect(41,13,3,10,1);
    display.drawLine(8,24,10,27,1);
    display.drawLine(73,24,71,27,1);
    display.drawLine(23,16,24,19,1);
    display.drawLine(58,16,57,19,1);

    display.display();
}