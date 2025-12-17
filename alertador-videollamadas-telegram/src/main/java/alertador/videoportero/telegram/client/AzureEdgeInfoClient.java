package alertador.videoportero.telegram.client;

import java.util.List;

import org.eclipse.microprofile.rest.client.annotation.RegisterClientHeaders;
import org.eclipse.microprofile.rest.client.inject.RegisterRestClient;

import jakarta.ws.rs.GET;
import jakarta.ws.rs.Path;
import alertador.videoportero.telegram.client.headerfactory.AzureEdgeInfoClientHeaderFactory;
import alertador.videoportero.telegram.dto.SyncedDevices;

@RegisterRestClient(configKey = "azure-edge")
@RegisterClientHeaders(AzureEdgeInfoClientHeaderFactory.class)
public interface AzureEdgeInfoClient {
    /**
     * Response example [{"macAddress":"AA:BB:CC:DD:EE:FF","mqttToken":"ad990d51-ed49-4565-8635-ac4ed2e1f41f","telegramAddress":"Ejemplo1"}]
    */
    @GET
    @Path("/sync-devices")
    List<SyncedDevices> syncDevices();
}