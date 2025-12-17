package alertador.videoportero.telegram.dto;

public record SyncedDevices(String macAddress, String mqttToken, String telegramAddress) {}