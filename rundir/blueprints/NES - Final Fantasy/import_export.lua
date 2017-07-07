
preImport = function()
    srcfile = io.open("srcfile","rb", true)
    
    return srcfile
end


preExport = function()
    srcfile = io.open("srcfile","rb", true)
    dstfile = io.open("dstfile","w+b", true)
    
    dstfile:write( srcfile:read("a") )
    
    return dstfile
end

postImExport = function(file)
    file:close()
end