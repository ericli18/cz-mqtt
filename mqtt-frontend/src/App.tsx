"use client";
import {
    AreaChart,
    Area,
    BarChart,
    Bar,
    LineChart,
    Line,
    XAxis,
    YAxis,
    CartesianGrid,
    Tooltip,
    ResponsiveContainer,
} from "recharts";
import {
    Card,
    CardContent,
    CardDescription,
    CardHeader,
    CardTitle,
} from "@/components/ui/card";

const mockData = {
    clientConnected: [
        { time: "00:00", count: 150 },
        { time: "04:00", count: 200 },
        { time: "08:00", count: 350 },
        { time: "12:00", count: 400 },
        { time: "16:00", count: 300 },
        { time: "20:00", count: 250 },
        { time: "23:59", count: 180 },
    ],
    topicSubscriptions: [
        { time: "00:00", count: 500 },
        { time: "04:00", count: 700 },
        { time: "08:00", count: 1200 },
        { time: "12:00", count: 1500 },
        { time: "16:00", count: 1300 },
        { time: "20:00", count: 1000 },
        { time: "23:59", count: 800 },
    ],
    mqttSessions: [
        { time: "00:00", count: 300 },
        { time: "04:00", count: 400 },
        { time: "08:00", count: 700 },
        { time: "12:00", count: 800 },
        { time: "16:00", count: 600 },
        { time: "20:00", count: 500 },
        { time: "23:59", count: 400 },
    ],
    messagesInOut: [
        { time: "00:00", in: 1000, out: 950 },
        { time: "04:00", in: 1500, out: 1400 },
        { time: "08:00", in: 3000, out: 2800 },
        { time: "12:00", in: 3500, out: 3300 },
        { time: "16:00", in: 2800, out: 2700 },
        { time: "20:00", in: 2000, out: 1900 },
        { time: "23:59", in: 1500, out: 1400 },
    ],
};

const MQTTDashboard = () => {
    return (
        <div className="mqtt-dashboard mx-auto max-w-7xl p-6">
            <h1 className="text-3xl font-bold mb-6">MQTT Dashboard</h1>
            <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
                <Card>
                    <CardHeader>
                        <CardTitle>Clients Connected</CardTitle>
                        <CardDescription>
                            Number of clients connected over time
                        </CardDescription>
                    </CardHeader>
                    <CardContent>
                        <ResponsiveContainer width="100%" height={300}>
                            <AreaChart data={mockData.clientConnected}>
                                <CartesianGrid strokeDasharray="3 3" />
                                <XAxis dataKey="time" />
                                <YAxis />
                                <Tooltip />
                                <Area
                                    type="monotone"
                                    dataKey="count"
                                    stroke="#8884d8"
                                    fill="#8884d8"
                                />
                            </AreaChart>
                        </ResponsiveContainer>
                    </CardContent>
                </Card>

                <Card>
                    <CardHeader>
                        <CardTitle>Topic Subscriptions</CardTitle>
                        <CardDescription>
                            Number of topic subscriptions over time
                        </CardDescription>
                    </CardHeader>
                    <CardContent>
                        <ResponsiveContainer width="100%" height={300}>
                            <BarChart data={mockData.topicSubscriptions}>
                                <CartesianGrid strokeDasharray="3 3" />
                                <XAxis dataKey="time" />
                                <YAxis />
                                <Tooltip />
                                <Bar dataKey="count" fill="#82ca9d" />
                            </BarChart>
                        </ResponsiveContainer>
                    </CardContent>
                </Card>

                <Card>
                    <CardHeader>
                        <CardTitle>MQTT Sessions</CardTitle>
                        <CardDescription>
                            Number of MQTT sessions over time
                        </CardDescription>
                    </CardHeader>
                    <CardContent>
                        <ResponsiveContainer width="100%" height={300}>
                            <LineChart data={mockData.mqttSessions}>
                                <CartesianGrid strokeDasharray="3 3" />
                                <XAxis dataKey="time" />
                                <YAxis />
                                <Tooltip />
                                <Line
                                    type="monotone"
                                    dataKey="count"
                                    stroke="#ffc658"
                                />
                            </LineChart>
                        </ResponsiveContainer>
                    </CardContent>
                </Card>

                <Card>
                    <CardHeader>
                        <CardTitle>Messages In/Out</CardTitle>
                        <CardDescription>
                            Number of messages received and sent over time
                        </CardDescription>
                    </CardHeader>
                    <CardContent>
                        <ResponsiveContainer width="100%" height={300}>
                            <LineChart data={mockData.messagesInOut}>
                                <CartesianGrid strokeDasharray="3 3" />
                                <XAxis dataKey="time" />
                                <YAxis />
                                <Tooltip />
                                <Line
                                    type="monotone"
                                    dataKey="in"
                                    stroke="#8884d8"
                                    name="Messages In"
                                />
                                <Line
                                    type="monotone"
                                    dataKey="out"
                                    stroke="#82ca9d"
                                    name="Messages Out"
                                />
                            </LineChart>
                        </ResponsiveContainer>
                    </CardContent>
                </Card>
            </div>
        </div>
    );
};

export default MQTTDashboard;
