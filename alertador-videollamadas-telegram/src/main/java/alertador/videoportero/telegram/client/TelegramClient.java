package alertador.videoportero.telegram.client;

import org.eclipse.microprofile.rest.client.inject.RegisterRestClient;
import org.jboss.resteasy.annotations.providers.multipart.MultipartForm;

import jakarta.ws.rs.Consumes;
import jakarta.ws.rs.GET;
import jakarta.ws.rs.POST;
import jakarta.ws.rs.Path;
import jakarta.ws.rs.QueryParam;
import jakarta.ws.rs.core.MediaType;
import alertador.videoportero.telegram.dto.Updates;
import alertador.videoportero.telegram.dto.Message;
import alertador.videoportero.telegram.dto.PhotoMessage;

@RegisterRestClient(configKey = "telegram-api")
public interface TelegramClient {
    @GET
    @Path("/getUpdates")
    Updates getUpdates(@QueryParam("offset") Integer offset);

    @POST
    @Path("/sendMessage")
    void sendMessage(Message message);

    @POST
    @Path("/sendPhoto")
    @Consumes(MediaType.MULTIPART_FORM_DATA)
    void sendPhoto(@MultipartForm PhotoMessage photoMessage);
}
