package alertador.videoportero.telegram.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

public record Message(
    @JsonProperty("message_id") 
    String messageId, 
    
    Chat chat
) {}