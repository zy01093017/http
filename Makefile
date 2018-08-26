http:http.cpp redis.h
	g++ -o $@ $^ -lhiredis
.PHONY:clean
clean:
	rm -f http
