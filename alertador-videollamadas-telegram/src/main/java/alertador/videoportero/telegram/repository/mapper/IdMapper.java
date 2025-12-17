package alertador.videoportero.telegram.repository.mapper;

import org.jboss.logging.Logger;

import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;

import java.util.List;

@ApplicationScoped
public class IdMapper {
    private Logger log;

    @Inject
    public IdMapper(Logger log) {
        this.log = log;
    }

    public Integer map(List<List<Object>> preresult) {
        if (preresult.isEmpty()) {
            log.debug("Id not found");
            return 0;
        }
        log.debug("Id found");
        return (Integer) preresult.get(0).get(0);
    }
}
