#include "Produtor.h"
#include <cstdlib>
#include <ctime>

void Produtor::inicializar() {
    *esp32WiFiClient = WiFiClient();
    *mqttClient = PubSubClient(*esp32WiFiClient);
    mqttClient->setServer(MQTT_BROKER, MQTT_PORT);
}

void Produtor::conectar() {
    String clientId = "ESP32ClientProjetoMonitoramento";

    unsigned long tempoInicio = millis();
    while(!(mqttClient->connected()) && (millis() - tempoInicio) < MQTT_TIMEOUT) {
        if(mqttClient->connect(clientId.c_str(), IO_USERNAME, IO_KEY)) {
            Serial.println("Conectado ao broker MQTT!");
        }
    }
}

bool Produtor::estaConectado() {
    return mqttClient->connected();
}

bool Produtor::publicarMensagemAlteracaoEstado(const MensagemAlteracaoEstado* mensagem) {
    String json = montarJSONAlteracaoEstado(mensagem);
    return mqttClient->publish(TOPICO_ALTERACAO_ESTADO, json.c_str(), true);
}

bool Produtor::publicarMensagemEntrada(const MensagemAcesso* mensagem) {
    String json = montarJSONAcesso(mensagem, "ENTRADA");
    return mqttClient->publish(TOPICO_ACESSO, json.c_str(), true);
}

bool Produtor::publicarMensagemSaida(const MensagemAcesso* mensagem) {
    String json = montarJSONAcesso(mensagem, "SAIDA");
    return mqttClient->publish(TOPICO_ACESSO, json.c_str(), true);
}

//=== UTILS: Obtém momemento atual no formato dd/MM/aaaa HH:mm:ss
const char* Produtor::obterMomentoAtual() {
    putenv("TZ=UTC+3:00");
    tzset();

    time_t now = time(0);
    tm localTime = *localtime(&now);

    char localTimeFormatado[20];
    strftime(localTimeFormatado, sizeof(localTimeFormatado), "%d/%m/%Y %H:%M:%S", &localTime);
    return localTimeFormatado;
}

String Produtor::montarJSONAlteracaoEstado(const MensagemAlteracaoEstado* mensagem) {
    String json;
    json.concat("{");
    json.concat("\"id\": " + String(mensagem->id) + ",");
    json.concat("\"nome\": \"" + String(mensagem->nome) + "\",");
    json.concat("\"estado\": \"" + String(mensagem->estado) + "\"");
    json.concat("}");
    return json;
}

String Produtor::montarJSONAcesso(const MensagemAcesso* mensagem, const char* tipo) {
    String json;
    json.concat("{");
    json.concat("\"idEstacao\": " + String(mensagem->idEstacao) + ",");
    json.concat("\"idAluno\": " + String(mensagem->aluno.id) + ",");
    json.concat("\"nomeAluno\": \"" + String(mensagem->aluno.nome) + "\",");
    json.concat("\"matriculaAluno\": \"" + String(mensagem->aluno.matricula) + "\",");
    json.concat("\"tipo\": \"" + String(tipo) + "\",");
    json.concat("\"momentoAcesso\": \"" + String(obterMomentoAtual()) + "\"");
    json.concat("}");
    return json;
}