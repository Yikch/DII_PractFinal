package alertador.videoportero.telegram.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

public record Update(
    @JsonProperty("update_id") 
    String updateId, 
    
    Message message
) {}