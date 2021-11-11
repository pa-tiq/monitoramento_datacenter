# Monitoramento de Datacenter 
Monitoramento do ambiente do Datacenter do 4º CTA utilizando ESP32

### Sumário 
* [Contextualização](#contextualização)
* [Objetivo](#objetivo)
* [Solução Apresentada](#solução-apresentada)
* [Lista de Materiais](#lista-de-materiais)

### Contextualização 

O datacenter, dentro do contexto do CMA, deve apresentar alta confiabilidade, por hospedar os servidores de todos os quartéis e também de órgãos de cidades da região amazônica, bem como do projeto da Amazônia Conectada.
Pela quantidade de equipamentos instalados e capacidade de processamento o ambiente deve ter um controle temperatura e umidade, para evitar possíveis danos ao equipamentos que acarretariam em gastos e manutenção para reativação dos servidores.

O monitoramento do ambiente, atualmente, é feito com a utilização de termômetros digitais comerciais que deve ser acompanhado no interior da sala do datacenter. Isso dificulta as tomada de ações preventivas, sendo somente notificado por temperatura interna do datacenter próximo a temperaturas críticas de operação.

### Objetivo

O objetivo do projeto foi a implementação de uma rede de sensores integrada por meio de conexão WiFi, de forma a realizar a medição de temperatura, umidade e presença no ambiente do datacenter, tanto localmente em um display instalado quanto remotamente por meio de uma página web em servidor.

### Solução Apresentada

Como forma de realizar a medição foram identificados os pontos críticos de aquecimento, que deveriam possuir o monitoramento continuado. Dessa forma os sensores seriam instalados na entrada da ventilação dos dois racks dos servidores e no rack de roteamento das unidades. Além disso, um M5Stack servirá como monitor central fazendo medição de temperatura afastado, agindo como parâmetro de comparação e fazendo o sensoriamento da entrada de pessoas na sala.

Os valores medidos em cada uma das estações, por meio de um ESP32 que se comunica por meio da rede WiFi com o M5Stack, sendo apresentados na tela localmente e também enviados para um servidor para monitoramento e acompanhamento remoto por meio de uma página http.

### Lista de Materiais

* 3x ESP32 
* 4x Sensores DHT11
* 1x Sensor HC-SR501 PIR
* 1x M5Stack
* 3x Protorboard
