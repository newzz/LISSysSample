--[[ CONSTANTS, STATES, AND SETUPS ]]
-- [[ this is just a sample so specific device names has been removed ]]

local RB_READ_HEADER, RB_READ_PARAMS, RB_READ_WBC, RB_READ_RBC, RB_READ_PLT, RB_READ_ID,RB_END = 1,2,3,4,5,6,7
local seq = {[0]=0x02,[1]=0x02,[2]=0xc5,[3]=0x38,[4]=0x01}
local attr;
local paramlabel ={
	"WBC", "LY#", "MO#", "GR#", "LY%","MO%", "GR%", "RBC", "HGB", "HCT", "MCV", "MCH", "MCHC", "RDWCV", "RDWSD", "PLT", "MPV", "PDW", "PCT"
}
local paramlabel_rev = {}
for k,v in ipairs(paramlabel) do
	paramlabel_rev[v] = k
end

local paramunits ={
	"10^9/uL", "10^9/uL", "10^9/uL", "10^9/uL", "%", "%", "%", "10^6/uL", "g/dL", "%", "fL", "Pg", "g/dL", "%", "fL", "10^9/uL", "fL", "fL", "%"
}

protocolname = "mystery"

--[[ IMPLEMENTATIONS ]]

--REQUIRED
function init(dev)
	devicename = dev
	fullname = protocolname.."@"..dev
	ptreename = protocolname.."@"..dev
	bytecount = 0
	checksum = 0
	state = RB_READ_HEADER
	attr = ptree.get(ptreename)	
	sys = LISSys.get()
	pdebug("module initialized successfully.\n")
end

--REQUIRED
function resetState()
	bytecount = 0
	checksum = 0
	next_state = RB_READ_HEADER
	attr = ptree.get(ptreename)	
end

--REQUIRED
function readbyte(b)		
	next_state = state
	if bytecount==0 then
		checksum=0
	elseif bytecount < 713 then 
		checksum = checksum ~ b		
	end
	if state==RB_READ_HEADER then				
		pdebug('.')
		if b==seq[bytecount] then			
			bytecount = bytecount+1			
			if bytecount==5 then
				next_state = RB_READ_PARAMS				
			end
		else			
			bytecount = 0
		end	
	elseif state==RB_READ_PARAMS then
		idx = bytecount - 5;					
		if (idx % 2 == 0)then
			parambyte = b			
		else
			label = 'param.'..paramlabel[math.floor(idx/2)+1]	
			paramfactor = ((i==8 or i==10) and 10) or 100
			paramval = ((parambyte << 8) + b)
			if paramval~=0x2A2A then
				attr:setNum(label, paramval/paramfactor)
			end
		end
		bytecount = bytecount + 1
		if bytecount == 5 + 19 * 2 then
			next_state = RB_READ_WBC
		end
	elseif state==RB_READ_WBC then
		idx = bytecount - 5 - 19 * 2		
		attr:setNum('wbc.idx'..idx,b)			
		bytecount = bytecount + 1
		if bytecount == 5 + 19 * 2 + 256 then
			next_state = RB_READ_RBC
		end
	elseif state==RB_READ_RBC then
		idx = bytecount - 5 - 19 * 2 - 256
		attr:setNum('rbc.idx'..idx,b)			
		bytecount = bytecount + 1
		if bytecount == 5 + 19 * 2 + 256 * 2 then
			next_state = RB_READ_PLT
		end
	elseif state==RB_READ_PLT then
		idx = bytecount - 5 - 19 * 2 - 256*2
		attr:setNum('plt.idx'..idx,b)			
		bytecount = bytecount + 1
		if bytecount == 5 + 19 * 2 + 256 * 2 + 128 then
			next_state = RB_READ_ID
		end			
	elseif state==RB_READ_ID then
		idx = bytecount-700
		if idx<0 then
			sampleID = 0
			bytecount = bytecount + 1
		elseif idx < 6 then
			sampleID = sampleID * 10 + b
			bytecount = bytecount + 1			
			if bytecount == 706 then
				next_state = RB_END
				attr:setNum('sampleID',sampleID)							
			end			
		end						
	elseif state==RB_END then
		if bytecount==712 then
			if b==0x03 then				
				pdebug("dataset received!\n")
			else 
				resetState()
			end			
		elseif bytecount==713 then
			if checksum==b then										
				attr:setNum('checksum',checksum)
				attr:setProp('device',devicename)
				attr:setProp('protocol',protocolname)
				attr:setNum('time', os.time())
				attr:setProp('time_s', os.date("%c"))
				attr:setProp('test','CBC')
				attr:setProp('desc','CBC')
				sys:submit(attr)
			else
				pdebug("checksum is incorrect: "..checksum.."!="..b.."\n")
			end
			resetState()
		end
		if bytecount>0 then
			bytecount = bytecount + 1
		end
	end	
	state = next_state	
end

--REQUIRED (used when a tabular data field edited from data frame)
function setParam(data, name, val)
	if paramlabel_rev[name] then		
		data:setNum('param.'..name,tonumber(val))
	end
end

--REQUIRED SYMBOL, implementation optional
orderform = [[ order form not supported ]]
--REQUIRED SYMBOL, implementation optional
function order(order_data,test_name) 
	pdebug('ordered '..test_name..'.\n')
end

--REQUIRED
function fillDataFrame(dataframe,data)
	dataframe:setTitle(data:getProp('test')..' record at '.. data:getProp('time_s'))
	dataframe:clearDataFields()
	for i=1,19 do
		local param = paramlabel[i]
		local key = 'param.'..param
		if data:has(key) then
			dataframe:appendDataField(param,tostring(data:getNum(key)),'-',paramunits[i])		
		end
	end
	dataframe:clearImages()
	dataframe:addImage('wbc_hist','draw_wbchist')
	dataframe:addImage('rbc_hist','draw_rbchist')
	dataframe:addImage('plt_hist','draw_plthist')
	dataframe:comment(data::getProp('comment'))
end

--[[ CALLBACK DRAW FUNCTIONS ]]
-- the format is f(dc,data)
-- C++ program will passed with appropriate objects that has been exposed to lua
-- with a limited set of functions, see dc:line, dc:text, etc in modelbindings.h

function draw_axes(dc,header)
	local w,h = dc:size()
	h = h-1
	dc:line(0,0,0,h)
	dc:line(0,h,w,h)
	dc:text(header,2,2)
end

function draw_wbchist(dc,data)
	local w,h = dc:size()
	dc:pen(0x00000000,1)
	draw_axes(dc,'wbc_hist')
	dc:pen(0,0)
	dc:brush(0x00FF0000,1)
	for i=1,256 do
		local val = data:getNum('wbc.idx'..(i-1))
		local x,y = i*(w-5)/256, val*(h-15)/256
		dc:rect(x,h-1-y,1,y);
	end
end
function draw_rbchist(dc,data)	
	local w,h = dc:size()
	dc:pen(0x00000000,1)
	draw_axes(dc,'rbc_hist')
	dc:pen(0,0)
	dc:brush(0x000000FF,1)
	for i=1,256 do
		local val = data:getNum('rbc.idx'..(i-1))
		local x,y = i*(w-5)/256, val*(h-15)/256
		dc:rect(x,h-1-y,1,y);
	end
end
function draw_plthist(dc,data)	
	local w,h = dc:size()
	dc:pen(0x00000000,1)
	draw_axes(dc,'plt_hist')
	dc:pen(0,0)
	dc:brush(0x0000FFFF,1)
	for i=1,128 do
		local val = data:getNum('plt.idx'..(i-1))
		local x,y = i*(w-5)/128, val*(h-15)/256
		dc:rect(x,h-1-y,2,y);
	end
end

--[[ database functions, ALL REQUIRED]]
db_alias = [[mystery]]
function db_create(lis2db)	
	local stmt = [[	
		CREATE TABLE IF NOT EXISTS mystery.record(
			rid INTEGER PRIMARY KEY ASC,
			uid INTEGER,
			"WBC" REAL, "LY#" REAL, "MO#" REAL, "GR#" REAL,
			"LY%" REAL, "MO%" REAL, "GR%" REAL, "RBC" REAL,
			"HGB" REAL, "HCT" REAL, "MCV" REAL, "MCH" REAL,
			"MCHC" REAL, "RDWCV" REAL, "RDWSD" REAL, "PLT" REAL,
			"MPV" REAL, "PDW" REAL, "PCT" REAL, 
			wbc_hist BLOB, rbc_hist BLOB, plt_hist BLOB
		);
	]]
	lis2db:exec(stmt)	
	lis2db:prepare('myst_insert',[[
		REPLACE INTO mystery.record( uid,"WBC", "LY#", "MO#", "GR#", "LY%","MO%", "GR%", "RBC", "HGB", "HCT", "MCV", "MCH", "MCHC", "RDWCV", "RDWSD", "PLT", "MPV", "PDW", "PCT", wbc_hist, rbc_hist, plt_hist )
		values                   ( @uid,@WBC, @LYn , @MOn , @GRn , @LYp , @MOp, @GRp , @RBC , @HGB , @HCT , @MCV , @MCH , @MCHC , @RDWCV , @RDWSD , @PLT , @MPV , @PDW , @PCT, @wbc_hist, @rbc_hist, @plt_hist  );
	]])
	lis2db:prepare('myst_delete',[[ DELETE FROM mystery.record WHERE uid=@uid; ]])
	lis2db:prepare('myst_findByUID',[[ SELECT * FROM mystery.record WHERE uid=@uid ]])
end

function db_drop(lis2db)
	lis2db:exec([[DROP TABLE mystery.record;]])
end

function db_insert(lis2db,data)		
	local inserter = lis2db:getStmt('myst_insert')	
	inserter:reset()
	inserter:bindInt(inserter:indexOf('@uid'),data:getNum('uid'))
	for i=1,19 do
		local param = paramlabel[i]
		local key = 'param.'..param
		local bindkey = '@'..param
		bindkey = bindkey:gsub('#','n')
		bindkey = bindkey:gsub('%%','p')
		if data:has(key) then			
			inserter:bindNum(inserter:indexOf(bindkey),data:getNum(key))
		end
	end		
	local wbc_hist,rbc_hist,plt_hist = LisBlob.alloc(256),LisBlob.alloc(256),LisBlob.alloc(128)	
	for i=1,256 do
		local histIdx = i-1
		wbc_hist:set(i,data:getNum('wbc.idx'..histIdx))
		rbc_hist:set(i,data:getNum('rbc.idx'..histIdx))		
		if i<=128 then
			plt_hist:set(i,data:getNum('plt.idx'..histIdx))
		end
	end	
	inserter:bindBlob(inserter:indexOf('@wbc_hist'),wbc_hist,256)
	inserter:bindBlob(inserter:indexOf('@rbc_hist'),rbc_hist,256)
	inserter:bindBlob(inserter:indexOf('@plt_hist'),plt_hist,128)
	inserter:step()
	LisBlob.free(wbc_hist)
	LisBlob.free(rbc_hist)
	LisBlob.free(plt_hist)	
end

function db_delete(lis2db,data)
	local deleter = lis2db:getStmt('myst_delete')	
	deleter:bindInt(deleter:indexOf('@uid'),data:getNum('uid'))
	deleter:step()	
end

function db_update(lis2db,data)
	db_insert(lis2db,data)
end

function db_fill(lis2db,data,hint)
	--fills first record found, specified by hint, into an empty data ptree
	
end

function db_find(lis2db,uid)	
	local finder = lis2db:getStmt('myst_findByUID')	
	finder:reset()
	inserter:bindInt(inserter:indexOf('@uid'),uid)	
end

function to_html(data)
	return [[
		<!doctype html>
		<html>
		</html>
	]]
end