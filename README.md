This project involves implementing an ”Emergency Service” platform subscription service, where users can subscribe to specific emergency channels based on their interests or needs, such as fire, medical, police, and natural disasters.
Subscribers can report emergencies and receive updates relevant to the channel’s topic, enabling effective community collaboration during crises.
For this purpose, I've implemented both a server and a client:
1. The server provides centralized STOMP (Simple-Text-Oriented-MessagingProtocol) server services, ensuring efficient communication between users, implemented in Java and support two modes of operation:
• Thread-Per-Client (TPC): Handles each client with a dedicated thread.
• Reactor: Uses an event-driven model for handling multiple clients efficiently.
The mode will be chosen based on arguments given at startup.

2. The client, implemented in C++, will allow users to interact with the
Emergency Service system. Each client handles logic for subscribing to channels, sending emergency reports, and receiving updates from the server.
All communication between the clients and the server adheres to the STOMP protocol, ensuring standardized messaging.
