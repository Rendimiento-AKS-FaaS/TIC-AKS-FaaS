from locust import HttpUser, task
import random

with open("movie_ids.txt") as f:
    movie_ids = [line.strip() for line in f]

class MediaUser(HttpUser):

    @task
    def read_page(self):

        movie_id = random.choice(movie_ids)

        self.client.get(
            f"/api/reading/page?movie_id={movie_id}",
            name="/api/reading/page"
        )
