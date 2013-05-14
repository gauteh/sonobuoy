function ggs = filterchannel (gs, chan)

l = size(gs,2);
I = false(1,l);

for k=1:l
  if strcmp(gs(k).ChannelIdentifier, chan)
    I(k) = true;
  end
end

ggs = gs(I);

end