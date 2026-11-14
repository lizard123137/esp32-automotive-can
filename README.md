# esp32-automotive-can

This project uses an `ESP32-S3` to connect to a vehicle `CAN BUS`. It is able to scan the existing traffic, as well as to send its own frames. It was built for an `IoT Devices` college course.

It is heavily inspired by the architecture of `.dbc` files.
As a demonstration of it working it can interface with a `BMW F30` gauge cluster.

![Interfacing with F30 cluster](/docs/images/f30-cluster-working.gif)

Current limitations and planned features, such as the fuel and temperature gauges, `OBD II` scanning and support for other automotive devices, are tracked under [Known limitations](docs/architecture.md#known-limitations).

## Documentation

The full project documentation lives in [`docs/`](docs/):

- [Overview](docs/index.md) – abstract, objectives, capabilities, hardware summary, repository structure, authors
- [Setup & Usage](docs/setup.md) – wiring, Wi-Fi configuration, building and flashing, using the web dashboard
- [Architecture](docs/architecture.md) – data paths, frame encoding, scheduling, concurrency model, modules, known limitations
- [API Reference](docs/api.md) – REST endpoints, WebSocket protocol and the C++ firmware API
- [BMW F30 Instrument Cluster](docs/F30-cluster.md) – checksum, rolling counters, update rates, frames sent by the cluster
- [BMW F30 CAN IDs](docs/F30-can-ids.md) – every implemented message and signal
- [DBC Files](docs/DBC-overview.md) – DBC messages and signals and how this project maps them
- [OBD-II](docs/OBD2-overview.md) – OBD-II background: modes, request/response frames and PIDs

The documentation is built with [MkDocs](https://www.mkdocs.org/) and the [Material](https://squidfunk.github.io/mkdocs-material/) theme. To host it as a local website at `http://127.0.0.1:8000/`:

```shell
pip install mkdocs-material
mkdocs serve
```

`mkdocs build --strict` renders the static site into `site/`, which is git-ignored.

## Authors

| Author | GitHub | LinkedIn |
| --- | --- | --- |
| **Michał Gagoś** | [lizard123137](https://github.com/lizard123137) | [michal-gagos](https://www.linkedin.com/in/michal-gagos/) |
| **Kamil Kostka** | [KaloeSan](https://github.com/KaloeSan) | [kamilkostka](https://www.linkedin.com/in/kamilkostka/) |

## Sources

A repository that helped us greatly with figuring out how to communicate with the cluster -  [CarCluster by r00li](https://github.com/r00li/CarCluster).
