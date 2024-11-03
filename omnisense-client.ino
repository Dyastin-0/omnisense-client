#include <WiFi.h>
#include <FirebaseClient.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <WebSocketsServer_Generic.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <map>
#include <vector>

#include "index.h"
#include "configPage.h"

#define databaseUrl "omnisense-17447-default-rtdb.asia-southeast1.firebasedatabase.app"
const String apiKey = "AIzaSyDI30Fd3AtxjZfCPC0QnaaQ68lUWe1_eK0";

struct Device {
	String name;
	uint8_t pin;
	bool state;
};

std::map<String, Device> devicesMap;

void asyncCB(AsyncResult &aResult);
void asyncCB1(AsyncResult &aResult);

DefaultNetwork network;

FirebaseApp app;

WiFiClientSecure ssl_client, ssl_client1, ssl_client2;

using AsyncClient = AsyncClientClass;

AsyncClient aClient(ssl_client1, getNetwork(network)), aClient2(ssl_client2, getNetwork(network));

RealtimeDatabase Database;

bool state;
bool taskListenerReady = false;

WebServer server(80); // Choose a port you want to use for the socket & web server
WebSocketsServer webSocket = WebSocketsServer(81);

bool isAuthenticated = false;
bool isConfigured = false;

String instancePath = "MyRoom";

std::vector<String> instances;

void handleAuth() {
	if (isConfigured) {
		server.send(200, "text/html", authPage);
	} else {
		server.send(200," text/html", configPage);
	}
}

void handleConfigureWifi() {
 	String body = server.arg("plain");
	
	StaticJsonDocument<200> config;
	deserializeJson(config, body);

	const char* ssid = config["ssid"];
	const char* password = config["password"];

	Serial.printf("[Omnisense] [Config] [WiFi] [SSID] %s\n", ssid);

	WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  byte tries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    if (tries++ > 30) {
      WiFi.mode(WIFI_AP);
      WiFi.softAP("omnisense", "config-it");
    	server.send(401, "Connection failed.");
      break;
    }
	}

	Serial.print("[Omnisense] [Wi-Fi] [IP] ");
	Serial.println(WiFi.localIP());

	isConfigured = true;
	if (MDNS.begin("omnisense")) Serial.println("[Omnisense] [MDNS] started.");
	MDNS.addService("http", "tcp", 80);

	server.send(200, "Success");
}

void asyncCB(AsyncResult &aResult) {
	if (aResult.available()) {
		RealtimeDatabaseResult &RTDB = aResult.to<RealtimeDatabaseResult>();
		if (RTDB.isStream()) {
			Serial.printf("[Omnisense] [%s] ----------------------------\n", aResult.uid().c_str());
			Firebase.printf("[Omnisense] [%s] [task] %s\n", aResult.uid().c_str(), aResult.uid().c_str());
			Firebase.printf("[Omnisense] [%s] [event] %s\n",aResult.uid().c_str(), RTDB.event().c_str());
			Firebase.printf("[Omnisense] [%s] [path] %s\n", aResult.uid().c_str(), RTDB.dataPath().c_str());
			Firebase.printf("[Omnisense] [%s] [data] %s\n", aResult.uid().c_str(), RTDB.to<const char *>());
			if (RTDB.event().c_str() != "keep-alive") {
				toggleRelay(RTDB.to<const char *>(), String(RTDB.dataPath().c_str()));
			}
		}
	}
	Firebase.printf("[Omnisense] [Heap]: %d\n", ESP.getFreeHeap());
}

void asyncCB1(AsyncResult &aResult) {
	if (aResult.available()) {
		RealtimeDatabaseResult &RTDB = aResult.to<RealtimeDatabaseResult>();
		if (RTDB.isStream()) {
			Serial.printf("[Omnisense] [%s] ----------------------------\n", aResult.uid().c_str());
			Firebase.printf("[Omnisense] [%s] [task] %s\n", aResult.uid().c_str(), aResult.uid().c_str());
			Firebase.printf("[Omnisense] [%s] [event] %s\n",aResult.uid().c_str(), RTDB.event().c_str());
			Firebase.printf("[Omnisense] [%s] [path] %s\n", aResult.uid().c_str(), RTDB.dataPath().c_str());
			Firebase.printf("[Omnisense] [%s] [data] %s\n", aResult.uid().c_str(), RTDB.to<const char *>());
			if (RTDB.event().c_str() != "keep-alive") {
				setInstances(RTDB.to<const char *>());
			}
		}
	}
	Firebase.printf("[Omnisense] [Heap]: %d\n", ESP.getFreeHeap());
}


void toggleRelay(const char* serializedDoc, String dataPath) {
	DynamicJsonDocument deserializeDoc(1024);
	deserializeJson(deserializeDoc, serializedDoc);

	if (deserializeDoc.is<JsonArray>()) {
		JsonArray array = deserializeDoc.as<JsonArray>();
		for (size_t i = 0; i < array.size(); i++) {
			JsonObject object = array[i];

			state = object["state"].as<bool>();
			String name = object["name"].as<String>();
			uint8_t pin = object["pin"].as<uint8_t>();

			pinMode(pin, OUTPUT);
			digitalWrite(pin, state);

			Device device;
			device.name = name;
			device.pin = pin;
			device.state = state;

			String path = "/" + String(i);

			devicesMap[path] = device;
		}
	} else {
		state = deserializeDoc["state"];
		devicesMap[dataPath].state = state;
		digitalWrite(devicesMap[dataPath].pin, state);
	}
}

void setInstances(const char* serializedDoc) {
	StaticJsonDocument<512> doc;
	deserializeJson(doc, serializedDoc);

	instances.clear();

	if (doc.is<JsonArray>()) {
		JsonArray jsonArray = doc.as<JsonArray>();
		for (JsonVariant value : jsonArray) {
			instances.push_back(value.as<String>());
		}
	} else {
		if (doc.is<JsonObject>()) {
			for (JsonPair kv : doc.as<JsonObject>()) {
				instances.push_back(kv.value().as<String>());
				Serial.printf("[Omnisense] [New Instance Added]: %s\n", kv.value().as<String>().c_str());
				break;
			}
		} else {
			Serial.println("[Omnisense] [Error] Expected JSON object for new instance.");
		}
		return;
	}

	StaticJsonDocument<512> sendDoc;
	JsonArray broadcastArray = sendDoc.createNestedArray("instances");

	for (const auto& instance : instances) {
		broadcastArray.add(instance);
	}

	String message;
	serializeJson(sendDoc, message);
	webSocket.broadcastTXT(message);

	Serial.printf("[Omnisense] [WebSocket] Sent instances to clients: %s\n", message.c_str());
}



void authenticateUser(const String &apiKey, const String &email, const String &password, uint8_t num) {
	if (isAuthenticated) return;
	if (ssl_client.connected()) ssl_client.stop();

	String host = "www.googleapis.com";

	if (ssl_client.connect(host.c_str(), 443) > 0) {
		String payload = "{\"email\":\"";
		payload += email;
		payload += "\",\"password\":\"";
		payload += password;
		payload += "\",\"returnSecureToken\":true}";

		String header = "POST /identitytoolkit/v3/relyingparty/verifyPassword?key=";
		header += apiKey;
		header += " HTTP/1.1\r\n";
		header += "Host: ";
		header += host;
		header += "\r\n";
		header += "Content-Type: application/json\r\n";
		header += "Content-Length: ";
		header += payload.length();
		header += "\r\n\r\n";

		if (ssl_client.print(header) == header.length()) {
			if (ssl_client.print(payload) == payload.length()) {
				unsigned long ms = millis();
				while (ssl_client.connected() && ssl_client.available() == 0 && millis() - ms < 5000) {
					delay(1);
				}
				ms = millis();
				while (ssl_client.connected() && ssl_client.available() && millis() - ms < 5000) {
					String line = ssl_client.readStringUntil('\n');
					if (line.length()) {
						isAuthenticated = line.indexOf("HTTP/1.1 200 OK") > -1;
						break;
					}
				}
				ssl_client.stop();
			}
		}
	}

	if (isAuthenticated) {
		UserAuth user_auth(apiKey, email, password);
		connectFirebase(user_auth, num);
		sendAuthResult(num, isAuthenticated);
	} else {
		sendAuthResult(num, isAuthenticated);
	}
}

void connectFirebase(UserAuth &user_auth, uint8_t num) {
	Firebase.printf("[Omnisense] [Firebase] [v] %s\n", FIREBASE_CLIENT_VERSION);

	Serial.println("[Omnisense] [Firebase] Initializing app...");

	ssl_client1.setInsecure();
	ssl_client2.setInsecure();

	initializeApp(aClient2, app, getAuth(user_auth), asyncCB, "authTask");
}

void initializeRTDB() {
	app.getApp<RealtimeDatabase>(Database);

	Database.url(databaseUrl);
	Database.setSSEFilters("get,put,patch,keep-alive,cancel,auth_revoked");

	String devicesPath = String(app.getUid()) + "/" + instancePath + "/devices";
	Serial.printf("[Omnisense] [Devices Path] %s\n", devicesPath.c_str());

	Database.get(aClient, devicesPath.c_str(), asyncCB, true, "State Listener");
	Database.get(aClient2, (String(app.getUid()) + "/instances").c_str(), asyncCB1, true, "Instance Fetcher");

	Serial.println("[Omnisense] [Firebase] Initialized with two streams.");
}

void stopActiveListeners() {
	aClient.stopAsync(true);
	aClient2.stopAsync(true);
}

void handleSetInstance() {
	if (isAuthenticated) {
		String selectedInstance = server.arg("plain");
		stopActiveListeners();
		instancePath = selectedInstance;
		Serial.printf("[Omnisense] [Instance]: %s\n", instancePath);
		taskListenerReady = false;
		server.send(200);
	} else {
		server.send(403);
	}
}

void sendAuthResult(uint8_t num, bool status) {
	DynamicJsonDocument doc(1024);
	JsonObject auth_result = doc.createNestedObject("auth_result");

	auth_result["isAuthenticated"] = status;
	auth_result["message"] = status ? "Success." : "Invalid email or password.";

	String response;
	serializeJson(doc, response);

	webSocket.sendTXT(num, response);
}

void webSocketEvent(const uint8_t& num, const WStype_t& type, uint8_t * payload, const size_t& length) {
	(void) length;
	switch (type) {
		case WStype_CONNECTED: {
			DynamicJsonDocument doc(1024);
			JsonObject auth_result = doc.createNestedObject("auth");

			auth_result["status"] = isAuthenticated ? "authenticated" : "not_authenticated";

			String response;
			serializeJson(doc, response);

			webSocket.sendTXT(num, response);
			break;
		}
		case WStype_TEXT: {
			DynamicJsonDocument doc(1024);
			deserializeJson(doc, payload);
			Serial.printf("[Omnisense] [Socket] [Message] %s\n", doc);

			if (doc.containsKey("auth_request")) {
				JsonObject authRequest = doc["auth_request"];
				const String email = authRequest["email"];
				const String password = authRequest["password"];

				authenticateUser(apiKey, email, password, num);
			}
			break;
		}
		default:
			break;
	}
}

void setup() {
	Serial.begin(115200);

	WiFi.mode(WIFI_AP);
	WiFi.softAP("omnisense", "config-it");

	Serial.print("[Omnisense] [Wi-Fi] [AP] [IP] ");
	Serial.println(WiFi.softAPIP());

	server.on("/", handleAuth);
	server.on("/config", HTTP_POST, handleConfigureWifi);
	server.on("/instance", HTTP_POST, handleSetInstance);
	server.begin();

	webSocket.onEvent(webSocketEvent);
	webSocket.begin();

	ssl_client.setInsecure();
}

void loop() {
	app.loop();
	Database.loop();
	server.handleClient();
	webSocket.loop();

	if (app.ready() && !taskListenerReady) {
		taskListenerReady = true;
		delay(250);
		initializeRTDB();
	}
}
