package alertador.videoportero.telegram.dto;

import org.jboss.resteasy.annotations.providers.multipart.PartFilename;
import org.jboss.resteasy.annotations.providers.multipart.PartType;

import jakarta.ws.rs.FormParam;
import jakarta.ws.rs.core.MediaType;

public class PhotoMessage {
    @FormParam("chat_id")
    @PartType(MediaType.TEXT_PLAIN)
    public String chatId;
    
    @FormParam("photo")
    @PartType(MediaType.APPLICATION_OCTET_STREAM)
    @PartFilename("suspect.jpg")
    public byte[] photo;
    
    @FormParam("caption")
    @PartType(MediaType.TEXT_PLAIN)
    public String caption;
    
    public PhotoMessage() {}
    
    public PhotoMessage(String chatId, byte[] photo, String caption) {
        this.chatId = chatId;
        this.photo = photo;
        this.caption = caption;
    }
}