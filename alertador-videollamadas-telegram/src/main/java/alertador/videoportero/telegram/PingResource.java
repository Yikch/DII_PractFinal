package alertador.videoportero.telegram;

import org.jboss.logging.Logger;

import jakarta.inject.Inject;
import jakarta.ws.rs.GET;
import jakarta.ws.rs.Path;
import jakarta.ws.rs.Produces;
import jakarta.ws.rs.core.MediaType;

@Path("/ping")
public class PingResource {
    private Logger log;

    @Inject
    public PingResource(Logger log) {
        this.log = log;
    }

    @GET
    @Produces(MediaType.TEXT_PLAIN)
    public String pong() {
        log.infof("Pingpong ball touching the table!");
        return "pong";
    }
}
