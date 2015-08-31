#
# A macro to help with pretty printing
#

AU_ALIAS([DE_STAT])
AC_DEFUN([DE_STAT], [
printf "    %-40.40s @<:@ " $1
if test $2 == "no"
then
    printf "\e@<:@31mNo\e@<:@0m @:>@\n"
else
    printf "\e@<:@32mYes\e@<:@0m @:>@\n"
fi
])
