//Yann Machado Bernardes
//Projeto Final de Curso
//Codigo fonte da celula de manufatura

//**********************************************************************************************
//  Bibliotecas
//**********************************************************************************************

  #include <Ethernet.h>
  #include <SPI.h>
  #include <PN532_HSU.h>
  #include "PN532.h"
  #include <Stepper.h>
  #include <Servo.h>
  #include <Pn532NfcReader.h>
  
//**********************************************************************************************
//  Definicoes
//**********************************************************************************************

  //Definicoes dos pinos do sensor rgb
  #define PINO_SENSOR_S0 49
  #define PINO_SENSOR_S1 47
  #define PINO_SENSOR_S2 46
  #define PINO_SENSOR_S3 48
  #define PINO_SENSOR_OUT 44
  #define PINO_SENSOR_VCC 42
  #define PINO_SENSOR_GND 43
  #define PINO_SENSOR_OE 45

  //Definicao numerica das cores
  #define VERMELHO 64
  #define VERDE 68
  #define AZUL 101

  //Definicao des estados da esteira e atuadores
  #define ATIVA 1
  #define DESATIVA 0

  //Definicao des estados dos produtos
  #define REPROVADO 0
  #define APROVADO 1
  #define INDEFINIDO 2
  
//**********************************************************************************************
//  Variaveis globais
//**********************************************************************************************

  //Estado do produto
  int produto = INDEFINIDO;

  //Retorno de estado dos leitores
  char readerStatusOn[] = "Online"; 
  char readerStatusOf[] = "Offline";
  uint32_t versiondata1;
  uint32_t versiondata2;
  uint32_t versiondata3;

  //Leitura de tag
  boolean success1;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;
  uint8_t id_dec = 0;

  int passo[20] = {0};

  //PNRD
  int8_t matrizIncidencia[] = {-1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                                1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                                0,  1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                                0,  1,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                                0,  1,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                                0,  0,  1,  1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                                0,  0,  0,  0,  1, -1,  0,  0,  0,  0,  0,  0,  0,  0,
                                0,  0,  0,  0,  0,  1, -1,  0,  0,  0,  0,  0,  0,  0,
                                0,  0,  0,  0,  0,  0,  1, -1,  0,  0,  0,  0,  0,  0,
                                0,  0,  0,  0,  0,  0,  0,  1, -1, -1,  0,  0,  0,  0,
                                0,  0,  0,  0,  0,  0,  0,  0,  1,  0, -1,  0,  0,  0,
                                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,
                                0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0, -1,  0,  0,
                                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1, -1,  0,
                                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1, -1,
                                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1};

  uint16_t vetorMarcacao[] = {1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};

  int contador = 1;

//**********************************************************************************************
//  Pre-setup
//**********************************************************************************************

  //********************************************************************************************
  //  1 - Ethernet Shield
  //********************************************************************************************

  //Configuracoes iniciais do servidor telnet
  byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
  IPAddress ip( 192, 168, 1, 25);
  IPAddress myDns(192, 168, 1, 1);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 0, 0);

  //Configuracao de porta e cliente
  EthernetServer server(23);
  EthernetClient client;

  //********************************************************************************************
  //  2 - RFID
  //********************************************************************************************

  //Leitor 1
  PN532_HSU pn532hsu1(Serial1);
  PN532 nfc1(pn532hsu1);

  //Leitor 2
  PN532_HSU pn532hsu2(Serial2);
  PN532 nfc2(pn532hsu2);

  //Leitor 3
  PN532_HSU pn532hsu3(Serial3);
  PN532 nfc3(pn532hsu3);

  //********************************************************************************************
  //  3 - Motor de passo
  //********************************************************************************************

  //Inicializa o motor de passo
  Stepper myStepper(500, 36, 38, 37, 39); //Valor passos por rev., pino 1, pino 3, pino 2, pino 4

  //********************************************************************************************
  //  4 - Servo
  //********************************************************************************************

  //Declaracao do objeto relativo ao servo motor
  Servo servoMotor;

  //********************************************************************************************
  //  5 - Sensor RGB
  //********************************************************************************************

  int qtdeVerm = 0;
  int qtdeVerd = 0;
  int qtdeAzul = 0;
  int cor = 0;

  //********************************************************************************************
  //  6 - PNRD
  //********************************************************************************************

  //Configuracao do leitor 2
  NfcAdapter pnrdR2 = NfcAdapter(pn532hsu2);
  Pn532NfcReader* reader2 = new Pn532NfcReader(&pnrdR2);
  Pnrd pnrd2 = Pnrd(reader2, 5, 4); 

  //Configuracao do leitor 3
  NfcAdapter pnrdR3 = NfcAdapter(pn532hsu3);
  Pn532NfcReader* reader3 = new Pn532NfcReader(&pnrdR3);
  Pnrd pnrd3 = Pnrd(reader3, 5, 4);

//**********************************************************************************************
//  Setup
//**********************************************************************************************

void setup()
{

  //********************************************************************************************
  //  0 - Arduino
  //********************************************************************************************

  //Inicializacao do serial monitor
  Serial.begin(9600);
  
  //********************************************************************************************
  //  1 - Ethernet Shield
  //********************************************************************************************

  //Inicia o shield e comeca a aceitar conexoes
  Ethernet.begin(mac, ip, myDns, gateway, subnet);
  server.begin();

  //********************************************************************************************
  //  2 - RFID
  //********************************************************************************************

  //1 - Inicializacao leitor 1
  nfc1.begin();
  Serial.print("Reader 1 Initialization...");
  versiondata1 = nfc1.getFirmwareVersion();
  if (! versiondata1)
  {
    Serial.println("Failed.");
  }
  else
  {
    Serial.println("OK");
    nfc1.SAMConfig();
  }
      
  //2 - Inicializacao leitor 2
  nfc2.begin();
  Serial.print("Reader 2 Initialization...");
  versiondata2 = nfc2.getFirmwareVersion();
  if (! versiondata2)
  {
    Serial.println("Failed.");
  }
  else
  {
    Serial.println("OK");
    nfc2.SAMConfig();
  }
      
  //3 - Inicializacao leitor 3
  nfc3.begin();
  Serial.print("Reader 3 Initialization...");
  versiondata3 = nfc3.getFirmwareVersion();
  if (! versiondata3)
  {
    Serial.println("Failed.");
  }
  else
  {
    Serial.println("OK");
    nfc3.SAMConfig();
  }

  //********************************************************************************************
  //  3 - Motor de passo
  //********************************************************************************************

  //Seleciona a velocidade do motor de passo 
  myStepper.setSpeed(60);

  //********************************************************************************************
  //  4 - Servo
  //********************************************************************************************

  //Seleciona o pino do servo e reinicia na posicao 0
  servoMotor.attach(33);
  servoMotor.write(0);

  //********************************************************************************************
  //  5 - Sensor RGB
  //********************************************************************************************

  //Define o modo de pinagem e seleciona a escala de frequencia de saida em 20%
  pinMode(PINO_SENSOR_S0, OUTPUT);
  pinMode(PINO_SENSOR_S1, OUTPUT);
  pinMode(PINO_SENSOR_S2, OUTPUT);
  pinMode(PINO_SENSOR_S3, OUTPUT);
  pinMode(PINO_SENSOR_OUT, INPUT);
  pinMode(PINO_SENSOR_VCC, OUTPUT);
  pinMode(PINO_SENSOR_GND, OUTPUT);
  pinMode(PINO_SENSOR_OE, OUTPUT);
  digitalWrite(PINO_SENSOR_S0,HIGH);
  digitalWrite(PINO_SENSOR_S1,LOW);
  digitalWrite(PINO_SENSOR_VCC,HIGH);
  digitalWrite(PINO_SENSOR_GND,LOW);
  digitalWrite(PINO_SENSOR_OE,LOW);

  //********************************************************************************************
  //  6 - Eletroima
  //********************************************************************************************

  //Configuracao do pino do eletroima
  pinMode(24, OUTPUT);
  digitalWrite(24, LOW);
  
}

//**********************************************************************************************
//  Loop
//**********************************************************************************************
void loop()
{
  
  //********************************************************************************************
  //  Lugar 0 e 1 - Ethernet Shield (Inicializacao do sistema e do processo)
  //********************************************************************************************  
  //Entrada do cliente e inicializacao do processo (Ativacao da esteira)
  EthernetClient newClient = server.accept();
  if (newClient)
  {
    if (!client)
    {
      client = newClient;
      client.println("***************************************************************************");
      client.println("                            Monitor de operacoes");
      client.println("***************************************************************************");
      client.println("");
      client.println(" Estado dos leitores:");
      client.print(" Leitor 1: ");
      versiondata1 ? client.println(readerStatusOn) : client.println(readerStatusOf);
      client.print(" Leitor 2: ");
      versiondata2 ? client.println(readerStatusOn) : client.println(readerStatusOf);
      client.print(" Leitor 3: ");
      versiondata3 ? client.println(readerStatusOn) : client.println(readerStatusOf);
      client.println("");
      client.println("***************************************************************************");
      client.println("");
      client.println("SISTEMA EM STAND-BY");   
      client.println("");
      client.println("  ---> Vetor de marcacao (M0) = [ 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ] (Inicial)");
      client.println(""); 
      client.println("1 - Processo iniciado, aguardando leitura da tag."); 
      fVetorMarcacao(1, vetorMarcacao);
      esteira(ATIVA); //Inicia a esteira
      }
    }
        
  //Processo de desconexao de cliente
  if (client && !client.connected())
  {
    Serial.print("disconnect client");
    client.stop();
    reset();
  }

  //********************************************************************************************
  //  Lugar 2, 3 ou 4 - (Leitura da tag e) Seleção de topo
  //********************************************************************************************  
  //Executa o processo de leitura da tag APENAS UMA VEZ e ja realiza a movimentacao do buffer
  if (((!vetorMarcacao[2]) || (!vetorMarcacao[3]) || (!vetorMarcacao[4])) && vetorMarcacao[1] && (success1 = nfc1.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength)))
  {
    //Zera a variavel de ID da tag
    id_dec = 0;

    //Processo de leitura da ID da tag
    for (int i = 0; i < uidLength; i++)
    {
      id_dec += uid[i];
    }
    
    //Selecao da cor com base no ID
    switch (id_dec)
    {
      case 64:
      //Exibe mensagem e vetor de marcacao do lugar 2 e seleciona o topo vermelho
      client.println("2 - Tag lida, escolhendo topo vermelho.");
      fVetorMarcacao(2, vetorMarcacao);
          seleciona(id_dec);
      break;
      
      case 68:
      //Exibe mensagem e vetor de marcacao do lugar 3 e seleciona o topo verde
      client.println("2 - Tag lida, escolhendo topo verde.");
      fVetorMarcacao(3, vetorMarcacao);
          seleciona(id_dec);
      break;
      
      case 101:
      //Exibe mensagem e vetor de marcacao do lugar 4 e seleciona o topo azul
      client.println("2 - Tag lida, escolhendo topo azul.");
      fVetorMarcacao(4, vetorMarcacao);
      seleciona(id_dec);
      break;
    }
  }
  //********************************************************************************************
  //  Lugar 5 - Parada da esteira
  //********************************************************************************************      
  //Realiza a parada da esteira
  if ((!vetorMarcacao[5]) && (vetorMarcacao[2] || vetorMarcacao[3] || vetorMarcacao[4]))
  {
    //Exibe mensagem e vetor de marcacao do lugar 5 e desativa a esteira
    client.println("3 - Topo escolhido, parando esteira.");
    fVetorMarcacao(5, vetorMarcacao);
    esteira(DESATIVA);
  }
  
  //********************************************************************************************
  //  Lugar 6 - Abertura da porta
  //********************************************************************************************    
  if ((!vetorMarcacao[6]) && vetorMarcacao[5])
  {
    //Exibe mensagem e vetor de marcacao do lugar 6 e realiza a abertura e fechamento (Lugar 7) de porta na mesma funcao 
    client.println("4 - Esteira parada, executando montagem.");
    fVetorMarcacao(6, vetorMarcacao);
    delay(1000);
    abertura();
  }
  
  //********************************************************************************************
  //  Lugar 7 - Fecha a porta (Realizado na funcao abertura() anterior) e aciona a esteira 
  //********************************************************************************************  
  if ((!vetorMarcacao[7]) && vetorMarcacao[6])
  {
    //Exibe mensagem e vetor de marcacao do lugar 7 e reativa a esteira
    client.println("5 - Montagem concluida, continuando processo.");   
    fVetorMarcacao(7, vetorMarcacao);
    esteira(ATIVA);
  }
  
  //********************************************************************************************
  //  Lugar 8 - Realizando escrita da tag
  //******************************************************************************************** 
  if ((!vetorMarcacao[8]) && vetorMarcacao[7])
  {
    //Definicao da rede de petri
    pnrd3.setIncidenceMatrix(matrizIncidencia);
    pnrd3.setTokenVector(vetorMarcacao);

    //Definicao da abordagem classica PNRD
    pnrd3.setAsTagInformation(PetriNetInformation::TOKEN_VECTOR);
    pnrd3.setAsTagInformation(PetriNetInformation::ADJACENCY_LIST);  
    
    //Exibe mensagem e vetor de marcacao do lugar 8 (A leitura ocorre adiante)
    client.println("6 - Realizando atualizacao da matriz de marcacao na tag.");     
    fVetorMarcacao(8, vetorMarcacao); 
  }
  
  //********************************************************************************************
  //  Lugar 9 - Parada da esteira e realizacao do teste de qualidade
  //********************************************************************************************
  //Realiza a parada da esteira e teste de qualidade, e faz a escrita na tag (Lugar 8)
  if ((!vetorMarcacao[9]) && vetorMarcacao[8] && (pnrd3.saveData() == WriteError::NO_ERROR))
  {
    //Transicao T7
    delay(500);

    //Exibe mensagem e vetor de marcacao do lugar 9 e desativa a esteira
    client.println("7 - Tag atualizada com sucesso, parando esteira e executando teste de qualidade.");     
    fVetorMarcacao(9, vetorMarcacao);  
    esteira(DESATIVA);  
    delay(1000);

    //Realiza o teste de qualidade e exibe o resultado
    if (id_dec == (cor = teste()))
    {
      produto = APROVADO; //Sinal Aprovado (T8)
      client.println("Produto Aprovado!");    
      client.println("");  
    }
    if (id_dec != (cor = teste()))
    {
      produto = REPROVADO; //Sinal Reprovado (T9)
      client.println("Produto Reprovado"); 
      client.println("");     
    }
  }
  
  //********************************************************************************************
  //  Lugar 10 ou 12 - Aciona a esteira e realiza a escrita na tag (Aprovado ou reprovado)
  //******************************************************************************************** 
  //Realiza a escrita na tag (Escrita feita no passo a seguir) mediante verificacao de estado aprovado ou reprovado
  if ((!vetorMarcacao[10]) && vetorMarcacao[9])
  {
    //Definicao da rede de petri
    pnrd3.setIncidenceMatrix(matrizIncidencia);
    pnrd3.setTokenVector(vetorMarcacao);

    //Definicao da abordagem classica PNRD
    pnrd3.setAsTagInformation(PetriNetInformation::TOKEN_VECTOR);
    pnrd3.setAsTagInformation(PetriNetInformation::ADJACENCY_LIST);  
      
    //Ativa a esteira (Independente do estado)
    esteira(ATIVA);

    //Exibe mensagem e vetor de marcacao do lugar 10 ou 12, dependendo da aprovacao ou nao da peca
    client.println("8 - Realizando atualizacao da matriz de marcacao na tag.");
    produto ? fVetorMarcacao(10, vetorMarcacao) : fVetorMarcacao(12, vetorMarcacao);
  }
  
  //********************************************************************************************
  //  Lugar 11 - Finalizacao do produto aprovado
  //******************************************************************************************** 
  if ((!vetorMarcacao[11]) && vetorMarcacao[10] && (pnrd2.saveData() == WriteError::NO_ERROR))
  {
    //Exibe mensagem e vetor de marcacao do lugar 11
    client.println("9 - Tag atualizada com sucesso, finalizando produto.");
    fVetorMarcacao(11, vetorMarcacao);

    //Para da esteira e exibe a mensagem final 
    delay(5000); //Transicao T10
    esteira(DESATIVA);
    client.print("ESTADO - Produto Finalizado");

    //Reseta o sistema apos o final SERVIDOR CAIRA
    reset();    
  }

  //********************************************************************************************
  //  Lugar 13 - Inicio do processamento de produtos reprovados (Aciona eletroima e atuador 2)
  //******************************************************************************************** 
  if ((!vetorMarcacao[13]) && vetorMarcacao[12] && (pnrd2.saveData() == WriteError::NO_ERROR))
  {
    //Exibe mensagem de que a tag foi atualizada com sucesso (Passo anterior [lugar 12]) e executa acionamento
    //do eletroima e do atuador
    client.println("9 - Tag atualizada com sucesso, executando desmontagem.");
    digitalWrite(24, HIGH);
    atuador(2, ATIVA);

    //Exibe mensagem e vetor de marcacao do lugar 13
    client.println("9.1 - Eletroima ativo, atuador 2 estendido.");
    fVetorMarcacao(13, vetorMarcacao);
  }
  
  //********************************************************************************************
  //  Lugar 14 - Desliga o eletroima, retrai o atuador 2 e aciona o 3
  //******************************************************************************************** 
  if ((!vetorMarcacao[14]) && vetorMarcacao[13])
  {
    //Desliga o eletroima, retrai o atuador 2 e aciona o 3
    delay(5000);
    digitalWrite(24, LOW);
    atuador(2, DESATIVA);   
    atuador(3, ATIVA);

    //Exibe mensagem e vetor de marcacao do lugar 14
    client.println("9.2 - Eletroima desativado, atuador 2 retraido, atuador 3 estendido, finalizando processo.");
    fVetorMarcacao(14, vetorMarcacao);
  }

  //********************************************************************************************
  //  Lugar 14 - Finalizacao do produto reprovado
  //******************************************************************************************** 
  if ((!vetorMarcacao[15]) && vetorMarcacao[14])
  {
    //Exibe mensagem e vetor de marcacao do lugar 15
    client.println("10 - Atuador 3 retraido, esteira parada, processo finalizado com erro(s)!");
    fVetorMarcacao(15, vetorMarcacao);
  }

  //********************************************************************************************
  //  Extra
  //********************************************************************************************  
  if (vetorMarcacao[15])
  {
    //Retrai o atuador, para a esteira e exibe a mensagem final 
    delay(10000);
    atuador(3, DESATIVA); 
    esteira(DESATIVA);
    client.print("ESTADO - Produto descartado");
    
    //Reseta o sistema apos o final SERVIDOR CAIRA
    reset();    
  }
}

//**********************************************************************************************
//  Funcoes
//**********************************************************************************************

  //********************************************************************************************
  //  Abertura da porta
  //********************************************************************************************

  //Funcao responsavel por abrir a porta no buffer
  void abertura(void)
  {
    for(int i = 0; i < 60; i++)
    {
      servoMotor.write(i); //Move a cada loop
      delay(1); //Espera 1 milis
    } 

    delay(3000); //Espera 3 segundos para fechar a porta
    
    for(int i = 60; i >= 0; i--)
    {
      servoMotor.write(i); //Move a cada loop
      delay(1); //Espera 1 milis
    }
  }
  
  //********************************************************************************************
  //  Selecao de topos
  //********************************************************************************************

  //Funcao responsavel por girar o buffer e abrir o portao (Valor padrao = vermelho(64))
  void seleciona(int id_tag) 
  {
    if (id_tag == 64)
    {
      delay(1250);
    }
    if (id_tag == 68) //Verde
    {
      myStepper.step(682);
    }
    if (id_tag == 101) //Azul
    {
      myStepper.step(-682);
    }
  }

  //********************************************************************************************
  //  Teste de cor
  //********************************************************************************************

  //Funcao le a cor do objeto e retorna o valor numerico da cor (id da tag correspondente)
  int teste(void)
  {
    int teste_cor = 0;
    digitalWrite(PINO_SENSOR_S2, LOW); // Pino S2 em nivel baixo
    digitalWrite(PINO_SENSOR_S3, LOW); // Pino S3 em nivel baixo
  
    if(digitalRead(PINO_SENSOR_OUT) == HIGH){ // Verifica o nivel logico no pino OUT do sensor
      qtdeVerm = pulseIn(PINO_SENSOR_OUT, LOW); // Le duracao do pulso na saida
    } else {
      qtdeVerm = pulseIn(PINO_SENSOR_OUT, HIGH); // Le duracao do pulso na saida
    }
      
    digitalWrite(PINO_SENSOR_S3, HIGH); // PINO S3 em nivel alto
  
    if(digitalRead(PINO_SENSOR_OUT) == HIGH){ // Verifica o nivel logico no pino OUT do sensor
      qtdeAzul = pulseIn(PINO_SENSOR_OUT, LOW); // Le duracao do pulso na saida
    } else {
      qtdeAzul = pulseIn(PINO_SENSOR_OUT, HIGH); // Le duracao do pulso na saida
    }
      
    digitalWrite(PINO_SENSOR_S2, HIGH); // Pino S2 em nivel alto
  
    if(digitalRead(PINO_SENSOR_OUT) == HIGH){ // Verifica o nivel logico no pino OUT do sensor
      qtdeVerd = pulseIn(PINO_SENSOR_OUT, LOW); // Le duracao do pulso na saida
    } else {
      qtdeVerd = pulseIn(PINO_SENSOR_OUT, HIGH); // Le duracao do pulso na saida
    }
  
    // Verifica se a cor vermelha foi detectada
    if (qtdeVerm < qtdeAzul && qtdeVerm < qtdeVerd){
      teste_cor = VERMELHO;
      Serial.println("Vermelho");
    }
    
    if (qtdeAzul < qtdeVerm && qtdeAzul < qtdeVerd) { // Verifica se a cor azul foi detectada
      teste_cor = AZUL;
      Serial.println("Azul");
    }
    
    if (qtdeVerd < qtdeVerm && qtdeVerd < qtdeAzul) { // Verifica se a cor verde foi detectada
      teste_cor = VERDE;
      Serial.println("Verde"); // Verde em nivel alto
    }
  
    delay(1000); // Aguarda 1000 milissegundos
  
    return teste_cor;
  }

  //********************************************************************************************
  //  Esteira
  //********************************************************************************************

  //Funcao para ativar ou desativar a esteira
  void esteira(int acao)
  {
    pinMode(32, OUTPUT);
    if (acao == ATIVA)
    {
      digitalWrite(32, HIGH);
    }
    if (acao == DESATIVA)
    {
      digitalWrite(32, LOW);
    }    
  }

  //********************************************************************************************
  //  Atuadores
  //********************************************************************************************

  //Funcao para ativar ou desativar os atuadores
  void atuador(int num_at, int acao)
  {
    pinMode(28, OUTPUT);
    pinMode(29, OUTPUT);
    if ((num_at == 2) && (acao == ATIVA))
    {
      digitalWrite(28, LOW);
      digitalWrite(29, HIGH);
    }
    if ((num_at == 2) && (acao == DESATIVA))
    {
      digitalWrite(28, HIGH);
      digitalWrite(29, HIGH);
    }    
    if ((num_at == 3) && (acao == ATIVA))
    {
      digitalWrite(28, HIGH);
      digitalWrite(29, LOW);
    }
    if ((num_at == 3) && (acao == DESATIVA))
    {
      digitalWrite(28, HIGH);
      digitalWrite(29, HIGH);
    }    
  }

  //********************************************************************************************
  //  Funcao reset Arduino
  //********************************************************************************************
  void reset(void)
  {
    pinMode(8, OUTPUT);
    digitalWrite(8, HIGH);
    delay(1);
    digitalWrite(8, LOW);
  }

  //********************************************************************************************
  //  Funcao vetor de marcacao
  //********************************************************************************************
  //Funcao calcula o vetor de marcacao e exibe-o aos clientes caso seja pedido
  void fVetorMarcacao(int lugar, uint16_t *vetor)
  {
    //Calcula o vetor de marcacao
    for (int i = 0; i <= 15; i++)
    {
      i == lugar ? vetor[i] = 1 : vetor[i] = 0; 
    }

    //Realiza a exibicao
    client.println(" ");
    client.print("  ---> Vetor de marcacao (M");
    client.print(contador);
    client.print(") = [ ");
    for (int j = 0; j <= 15; j++)
      {
         client.print(vetor[j]);
         client.print(" ");
      }
      
    client.println("]"); 
    client.println("");
    contador++;
  }
