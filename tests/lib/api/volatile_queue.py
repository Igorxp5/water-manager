from asyncio import Queue

class VolatileQueue(Queue):
    """A subclass of Queue that removes the oldest added entry when putting in full 
    queue to leave free space.
    """

    def _put(self, item):
        if self.full():
            self._queue.popleft()
        self._queue.append(item)
