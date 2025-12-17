package alertador.videoportero.telegram.client.headerfactory;

import org.eclipse.microprofile.config.inject.ConfigProperty;
import org.eclipse.microprofile.rest.client.ext.ClientHeadersFactory;
import org.jboss.logging.Logger;

import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;
import jakarta.ws.rs.core.MultivaluedHashMap;
import jakarta.ws.rs.core.MultivaluedMap;

@ApplicationScoped
public class AzureEdgeInfoClientHeaderFactory implements ClientHeadersFactory {
    @ConfigProperty(name = "azure.edge.api.key")
    String apiKey;

    private Logger log;

    @Inject
    public AzureEdgeInfoClientHeaderFactory(Logger log) {
        this.log = log;
    }

    @Override
    public MultivaluedMap<String, String> update(MultivaluedMap<String, String> incomingHeaders, MultivaluedMap<String, String> outgoingHeaders) {
        MultivaluedMap<String, String> newHeaders = new MultivaluedHashMap<>();
        if (apiKey != null && !apiKey.isEmpty()) {
            newHeaders.add("x-edge-api-key", apiKey);
        } else {
            log.error("ERROR: azure.edge.api.key property not defined");
        }
        return newHeaders;
    }
}
