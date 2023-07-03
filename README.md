# c-basic-auth
The basic type authentication in the C programming language.

## Getting Started
Clone this project and open project directory from your terminal.

Following command strat a auth.service in daemon:
```
sudo make install
```

Result of this command:

![image](https://user-images.githubusercontent.com/32206555/166876745-b01bcc73-dd02-4d3f-a775-ed0a8998713e.png)


Check status of the tour service:
```
sudo systemctl status auth.service
```

Will be get following result:

![image](https://user-images.githubusercontent.com/32206555/166876823-346e977b-4217-4184-971b-589a1821acc1.png)


After you can open [localhost:8080](http://localhost:8080) on tyour browser to authentication:

![image](https://user-images.githubusercontent.com/32206555/166877345-8966ed2b-f88a-483c-872e-778de3a3d9bf.png)


After authentication will be open following page with tree structure of root directory:

![image](https://user-images.githubusercontent.com/32206555/166877555-5e436eca-c78e-4bb2-aa83-03bb02912501.png)

And check logs of your servie:

![image](https://user-images.githubusercontent.com/32206555/166877183-53c425a4-bd2a-49f6-94fa-250a8bfb669d.png)


You can go to some directory via URL adress navigation:

![image](https://user-images.githubusercontent.com/32206555/166877813-d4f049ae-9f0f-4441-8002-ddf3801de439.png)

If you haven't accees to this directory then you will get this page:

![image](https://user-images.githubusercontent.com/32206555/166878041-06b66dd1-c14d-4f55-ab03-1cef37d51ea5.png)


If you want stop the auth.service just execute following command:
```
sudo make uninstall
```

And for auto delete generated files execute following command:
```
sudo make clean
```

Thanks for you attention!
