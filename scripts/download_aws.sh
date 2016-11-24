ips="
ec2-35-164-59-153.us-west-2.compute.amazonaws.com
ec2-35-163-178-17.us-west-2.compute.amazonaws.com
ec2-35-164-69-4.us-west-2.compute.amazonaws.com
ec2-35-164-91-124.us-west-2.compute.amazonaws.com
ec2-35-161-49-182.us-west-2.compute.amazonaws.com
ec2-35-162-196-234.us-west-2.compute.amazonaws.com
ec2-35-160-161-7.us-west-2.compute.amazonaws.com
ec2-35-164-79-78.us-west-2.compute.amazonaws.com
ec2-35-164-79-78.us-west-2.compute.amazonaws.com
ec2-35-162-246-239.us-west-2.compute.amazonaws.com
"
user="ubuntu"

for ip in $ips; do
  scp -i "$1" -r "$user"@"$ip":/home/"$user"/crobby/resultados resultados
done
