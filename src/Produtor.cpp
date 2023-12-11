#include "Produtor.h"

void Produtor::inicializar() {
    mqttClient = PubSubClient(esp32WiFiClient);
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    randomSeed(124718274);
    clientId.concat(String(random(1, 800000)));
}

void Produtor::conectar() {
    unsigned long tempoInicio = millis();
    Serial.println("MQTT: " + String(mqttClient.state()));
    while(!(mqttClient.connected()) && (millis() - tempoInicio) < MQTT_TIMEOUT) {
        if(mqttClient.connect(clientId.c_str(), IO_USERNAME, IO_KEY)) {
            Serial.println("Conectado ao broker MQTT!");
        }
        Serial.println("MQTT: " + String(mqttClient.state()));
        delay(1000);
    }
}

bool Produtor::estaConectado() {
    Serial.print("Estado MQTT (estaConectado): ");
    Serial.println(mqttClient.state());
    return mqttClient.connected();
}

bool Produtor::manterConexao() {
    return mqttClient.loop();
}

bool Produtor::publicarMensagemAlteracaoEstado(const MensagemAlteracaoEstado mensagem) {
    String json = montarJSONAlteracaoEstado(mensagem);
    return mqttClient.publish(TOPICO_ALTERACAO_ESTADO, json.c_str(), true);
}

bool Produtor::publicarMensagemEntrada(const MensagemAcesso mensagem) {
    String json = montarJSONAcesso(mensagem, "ENTRADA");
    return mqttClient.publish(TOPICO_ACESSO, json.c_str(), true);
}

bool Produtor::publicarMensagemSaida(const MensagemAcesso mensagem) {
    String json = montarJSONAcesso(mensagem, "SAIDA");
    return mqttClient.publish(TOPICO_ACESSO, json.c_str(), true);
}

String Produtor::montarJSONAlteracaoEstado(const MensagemAlteracaoEstado mensagem) {
    String json;
    json.concat("{");
    json.concat("\"id\": ");
    json.concat(String(mensagem.id));
    json.concat(",");
    json.concat("\"nome\": \"");
    json.concat(String(mensagem.nome));
    json.concat("\",");
    json.concat("\"estado\": \"");
    json.concat(String(mensagem.estado));
    json.concat("\"}");
    return json;
}

String Produtor::montarJSONAcesso(const MensagemAcesso mensagem, String tipo) {
    String json;
    json.concat("{");
    json.concat("\"idEstacao\": ");
    json.concat(String(mensagem.idEstacao));
    json.concat(",");
    json.concat("\"idAluno\": ");
    json.concat(String(mensagem.aluno.id));
    json.concat(",");
    json.concat("\"nomeAluno\": \"");
    json.concat(String(mensagem.aluno.nome));
    json.concat("\",");
    json.concat("\"matriculaAluno\": \"");
    json.concat(String(mensagem.aluno.matricula));
    json.concat("\",");
    json.concat("\"tipo\": \"");
    json.concat(String(tipo));
    json.concat("\"}");
    return json;
}