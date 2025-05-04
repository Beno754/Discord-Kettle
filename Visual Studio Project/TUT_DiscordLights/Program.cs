namespace TUT_DiscordLights
{
    internal class Program
    {

        // Your Discord bot token here
        public static string YOUR_TOKEN_HERE = "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLM_-NOPQR";

        //Entry point
        static void Main(string[] args)
        {
            MainAsync().GetAwaiter().GetResult(); // Never returns a result to keep the program running
        }







        static async Task MainAsync()
        {

            // Setup Discord connection
            var discord = new DiscordClient(new DiscordConfiguration()
            {
                Token = YOUR_TOKEN_HERE, // Bot token
                TokenType = TokenType.Bot, // User is bot account
                Intents = DiscordIntents.AllUnprivileged // Access rights
            });


            discord.MessageCreated += MessageCreatedHandler; // Subscribe to the message created event

            
            await discord.ConnectAsync(); // Connect to the server
            await Task.Delay(-1); // Never return to keep program running
        }

        // Global variables for client messaging
        public static byte[] buffer = new byte[100]; // buffer
        public static NetworkStream clientStream = null; // global reference for stream object


        // Message Event Callback
        private static Task MessageCreatedHandler(DiscordClient sender, MessageCreateEventArgs e)
        {                    
            if (e.Guild?.Id == 123456789012345678)  // Make sure we only listen to messages from a specific server. We could also check for userID
                Console.WriteLine("Checking Server OK");

            //check the command string matches
            if (e.Message.Content == "-kettle")
            {
                Console.WriteLine("Polly put the kettle on!"); // Console debug                

                kettleOn(e); // Run kettle procedure
            }

            Console.WriteLine("---");
            return Task.CompletedTask; // Return from method
        }




        // Send command to Arduino -> Kettle
        static void kettleOn(MessageCreateEventArgs e)
        {

            TcpClient client = new TcpClient("192.168.1.170", 8001); // Client object
            clientStream = client.GetStream(); // Get a stream object from client

            string msg = "k1"; // Message to send


            byte[] temp = Encoding.Default.GetBytes(msg); // Convert string variable into byte array for streaming
            clientStream.Write(temp, 0, temp.Length); // Write data to stream

            buffer = new byte[100]; // buffer to read into
            clientStream.BeginRead(buffer, 0, buffer.Length, ReadASync, e); // Read the response, handled in an ASYNC method


            clientStream.Flush(); //clean up the stream
        }





        // Async method to handle reading the response from client
        static void ReadASync(IAsyncResult ar) 
        {
            MessageCreateEventArgs e = (MessageCreateEventArgs)ar.AsyncState; // MessageCreatedEvent arguments

            int bytesRead = clientStream.EndRead(ar); // get number of bytes recieved as a result

            //read the amount of bytes into message
            string msgRecieved = Encoding.Default.GetString(buffer, 0, bytesRead); // encode byte array into a string
            Console.WriteLine("Recieved: " + msgRecieved); // write message to Console


            // Case for response code
            switch (msgRecieved)
            {

                case "r1": // OK Handshake, continue listening for Completion message
                    Console.WriteLine("Ok kettle response - waiting for completion");
                    e.Channel.SendMessageAsync("Putting kettle on for you m'Lord!").ConfigureAwait(false);
                    buffer = new byte[100]; // buffer to read into
                    clientStream.BeginRead(buffer, 0, buffer.Length, ReadASync, e); // Read the next response, handled in an ASYNC method
                    break;

                case "r2": // Completion message, no need to listen for anything else
                    Console.WriteLine("Kettle Done!");
                    e.Channel.SendMessageAsync("Kettle is boilt! ☕").ConfigureAwait(false);
                    clientStream.Close(); // close connection to the client
                    break;

                case "e1": // Error with scale, no need to listen for anything else
                    Console.WriteLine("Scale issue!");
                    e.Channel.SendMessageAsync("2.2/10 Not enough water! 💧").ConfigureAwait(false);
                    clientStream.Close(); // close connection to the client
                    break;

                default:
                    break;
            }

        }





    }// Program
}// Namespace
