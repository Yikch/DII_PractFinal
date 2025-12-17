package alertador.videoportero.telegram.dto;

import java.util.List;

public record Updates (boolean ok, List<Update> result) {}