package alertador.videoportero.telegram.dto;

public record ImageAlert(
    Integer id,
    byte [] image,
    String topic,
    String telegramAddress,
    Integer sent
) {}